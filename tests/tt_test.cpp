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
    Board board1 = FEN::parse("8/8/8/8/8/8/8/k1K3N1 w - - 0 1");
    EXPECT_EQ(board1.hash, Zobrist::generate_hash(board1));

    // Position after g1-f3-g1
    Board board2 = board1;
    for (const char* uci : {"g1f3", "a1a2", "f3g1", "a2a1"}) {
      board2.makeMove(PrintingHelpers::uci_to_move(uci, board2));
      EXPECT_EQ(board2.hash, Zobrist::generate_hash(board2)) << uci;
    }

    EXPECT_EQ(board1.hash, board2.hash);
    EXPECT_EQ(Zobrist::generate_hash(board1), Zobrist::generate_hash(board2));
}

TEST_F(TranspositionTableTest, SearchConsistency) {
    Board board1 = FEN::parse("8/8/8/8/8/8/8/k1K3N1 w - - 0 1");

    Board board2 = board1;
    for (const char* uci : {"g1f3", "a1a2", "f3g1", "a2a1"})
      board2.makeMove(PrintingHelpers::uci_to_move(uci, board2));

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

TEST_F(TranspositionTableTest, WarmRootSearchReusesExactEntry) {
    Board board = FEN::parse();
    TranspositionTable tt;

    FoChess::iterative_deepening(5, board, tt);
    const int first_score = FoChess::g_search_stats.best_root_score.load();
    const Move first_move = FoChess::g_search_stats.best_move.load();
    const uint64_t first_nodes = FoChess::g_search_stats.node_count.load();
    ASSERT_GT(first_nodes, 0);

    FoChess::iterative_deepening(5, board, tt);
    EXPECT_EQ(FoChess::g_search_stats.best_root_score.load(), first_score);
    EXPECT_EQ(FoChess::g_search_stats.best_move.load(), first_move);
    EXPECT_EQ(FoChess::g_search_stats.node_count.load(), 0);
}

TEST_F(TranspositionTableTest, ClusterKeepsCollidingPositions) {
    TranspositionTable tt(1);
    constexpr uint64_t bucket_count = (1024 * 1024) / sizeof(TTCluster);
    const Move first_move(A1, A2);
    const Move second_move(H8, H7);

    tt.store(7, 10, first_move, 4, TT_EXACT);
    tt.store(7 + bucket_count, 20, second_move, 5, TT_BETA);

    const TTEntry* first = tt.probe(7);
    const TTEntry* second = tt.probe(7 + bucket_count);
    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);
    EXPECT_EQ(first->best_move, first_move);
    EXPECT_EQ(second->best_move, second_move);
}

TEST_F(TranspositionTableTest, QuiescenceEntryCannotEvictDeeperEntry) {
    TranspositionTable tt(1);
    const Move deep_move(E2, E4);

    tt.store(11, 42, deep_move, 7, TT_EXACT);
    tt.store_quiescence(11, -20, Move(A2, A3), TT_ALPHA);

    const TTEntry* entry = tt.probe(11);
    ASSERT_NE(entry, nullptr);
    EXPECT_EQ(entry->score, 42);
    EXPECT_EQ(entry->best_move, deep_move);
    EXPECT_EQ(entry->depth, 7);
    EXPECT_EQ(entry->flag, TT_EXACT);
}
