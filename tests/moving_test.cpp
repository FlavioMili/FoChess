// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#include <gtest/gtest.h>

#include <array>

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

TEST_F(MovingTest, SpecialMovesAndCastlingChangesMaintainHash) {
    struct Case {
      const char* fen;
      const char* uci;
    };
    const std::array<Case, 6> cases = {{
        {"r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1", "e1f1"},
        {"r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1", "a1a2"},
        {"r3k2r/8/8/8/8/8/r7/R3K2R b KQkq - 0 1", "a2a1"},
        {"r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1", "e1g1"},
        {"4k3/P7/8/8/8/8/8/4K3 w - - 0 1", "a7a8q"},
        {"4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1", "e5d6"},
    }};

    for (const Case& test : cases) {
      Board board = FEN::parse(test.fen);
      const Move move = PrintingHelpers::uci_to_move(test.uci, board);
      ASSERT_TRUE(board.moveExists(move)) << test.uci;
      ASSERT_TRUE(board.isLegalMove(move)) << test.uci;
      board.makeMove(move);
      EXPECT_EQ(board.hash, Zobrist::generate_hash(board)) << test.uci;
    }
}

TEST_F(MovingTest, NullMoveMaintainsHashAndBoardState) {
    Board board = FEN::parse(
        "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 7 2");
    const auto pieces = board.pieces;
    const auto occupancy = board.occupancy;
    const Bitboard all_pieces = board.allPieces;
    const CastlingRights castling = board.castling;
    const uint16_t halfmove_clock = board.halfMoveClock;
    const uint16_t fullmove_number = board.fullMoveNumber;

    board.makeNullMove();

    EXPECT_EQ(board.sideToMove, BLACK);
    EXPECT_EQ(board.enPassant, NO_SQUARE);
    EXPECT_EQ(board.pieces, pieces);
    EXPECT_EQ(board.occupancy, occupancy);
    EXPECT_EQ(board.allPieces, all_pieces);
    EXPECT_EQ(board.castling.whiteKingside, castling.whiteKingside);
    EXPECT_EQ(board.castling.whiteQueenside, castling.whiteQueenside);
    EXPECT_EQ(board.castling.blackKingside, castling.blackKingside);
    EXPECT_EQ(board.castling.blackQueenside, castling.blackQueenside);
    EXPECT_EQ(board.halfMoveClock, halfmove_clock);
    EXPECT_EQ(board.fullMoveNumber, fullmove_number);
    EXPECT_EQ(board.hash, Zobrist::generate_hash(board));
}
