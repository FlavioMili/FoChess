// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#include <gtest/gtest.h>
#include "movegen.h"
#include <array>
#include <string>
#include "board.h"
#include "fen.h"
#include "helpers.h"
#include "magic.h"
#include "zobrist.h"

class MoveGenTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        Bitboards::init_magic_tables();
        Zobrist::init_zobrist_keys();
    }

    void TestBoardGeneration(const std::string& fen, size_t expected_moves) const {
        Board board = FEN::parse(fen);
        std::array<Move, MAX_MOVES> moves;
        size_t n_moves = MoveGen::generate_all(board, moves);
        EXPECT_EQ(n_moves, expected_moves) << "Failed for FEN: " << fen;
    }
};

TEST_F(MoveGenTest, StandardPositions) {
    TestBoardGeneration("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -", 20);
    TestBoardGeneration("8/8/8/1pP3n1/8/5k2/5N2/4K3 w - b6 0 2", 11);
    TestBoardGeneration("8/4k3/4q3/8/8/4N3/8/4K3 w - - 0 2", 5);
    TestBoardGeneration("8/8/8/8/8/1q6/8/K1k5 w - - 0 1", 0);
    TestBoardGeneration("8/4k2P/8/Pp4p1/8/8/8/R3K2R w KQ - 0 1", 25);
    TestBoardGeneration("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 b - - 0 1 ", 15);
}
