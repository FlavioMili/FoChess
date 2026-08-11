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

    uint64_t previous_hash = board.hash;
    for (const char* uci : {"e2e4", "e7e5", "g1f3", "b8c6"}) {
      const Move move = PrintingHelpers::uci_to_move(uci, board);
      ASSERT_TRUE(board.moveExists(move)) << uci;
      ASSERT_TRUE(board.isLegalMove(move)) << uci;
      board.makeMove(move);
      EXPECT_NE(previous_hash, board.hash) << uci;
      EXPECT_EQ(board.hash, Zobrist::generate_hash(board)) << uci;
      previous_hash = board.hash;
    }
}
