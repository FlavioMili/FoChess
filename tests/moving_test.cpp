// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#include <gtest/gtest.h>
#include "board.h"
#include "fen.h"
#include "helpers.h"
#include "types.h"
#include "zobrist.h"
#include "magic.h"

class MovingTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        Bitboards::init_magic_tables();
        Zobrist::init_zobrist_keys();
    }
};

TEST_F(MovingTest, HashChanges) {
    std::string startFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -";
    Board board = FEN::parse(startFEN);

    uint64_t hash1 = Zobrist::generate_hash(board);

    board.makeMove(Move(Square::E2, Square::E3));
    uint64_t hash2 = Zobrist::generate_hash(board);
    EXPECT_NE(hash1, hash2);

    board.makeMove(Move(Square::E3, Square::E8));
    uint64_t hash3 = Zobrist::generate_hash(board);
    EXPECT_NE(hash2, hash3);
}
