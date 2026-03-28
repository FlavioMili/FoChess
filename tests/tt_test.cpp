// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#include <gtest/gtest.h>
#include "tt.h"
#include "board.h"
#include "fen.h"
#include "helpers.h"
#include "search.h"
#include "zobrist.h"
#include "magic.h"

class TranspositionTableTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        Zobrist::init_zobrist_keys();
        Bitboards::init_magic_tables();
    }
};

TEST_F(TranspositionTableTest, HashConsistency) {
    // Initial position
    Board board1 = FEN::parse("8/8/8/8/8/8/8/k1K5 w - - 0 1");
    board1.pieces[WHITE][KNIGHT] |= (1ULL << Square::G1);
    board1.updateOccupancy();

    // Position after g1-f3-g1
    Board board2 = board1;
    board2.makeMove(PrintingHelpers::uci_to_move("g1f3", board1));
    board2.makeMove(PrintingHelpers::uci_to_move("a1a2", board1));  // dummy move
    board2.makeMove(PrintingHelpers::uci_to_move("f3g1", board1));
    board2.makeMove(PrintingHelpers::uci_to_move("a2a1", board1));  // dummy move

    EXPECT_EQ(Zobrist::generate_hash(board1), Zobrist::generate_hash(board2));
}

TEST_F(TranspositionTableTest, SearchConsistency) {
    Board board1 = FEN::parse("8/8/8/8/8/8/8/k1K5 w - - 0 1");
    board1.pieces[WHITE][KNIGHT] |= (1ULL << Square::G1);
    board1.updateOccupancy();

    Board board2 = board1;
    board2.makeMove(PrintingHelpers::uci_to_move("g1f3", board1));
    board2.makeMove(PrintingHelpers::uci_to_move("a1a2", board1));  // dummy move
    board2.makeMove(PrintingHelpers::uci_to_move("f3g1", board1));
    board2.makeMove(PrintingHelpers::uci_to_move("a2a1", board1));  // dummy move

    TranspositionTable tt;

    FoChess::iterative_deepening(5, board1, tt);
    int score1 = FoChess::g_search_stats.best_root_score.load();
    Move move1 = FoChess::g_search_stats.best_move.load();

    FoChess::iterative_deepening(5, board2, tt);
    int score2 = FoChess::g_search_stats.best_root_score.load();
    Move move2 = FoChess::g_search_stats.best_move.load();

    EXPECT_EQ(score1, score2);
    EXPECT_EQ(move1, move2);
}
