// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#include <gtest/gtest.h>
#include "board.h"
#include "helpers.h"
#include "fen.h"
#include "magic.h"
#include "zobrist.h"
#include <string>

class FENTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        Bitboards::init_magic_tables();
        Zobrist::init_zobrist_keys();
    }
};

TEST_F(FENTest, RoundTrip) {
    // Standard starting position FEN
    std::string startFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -";
    Board board = FEN::parse(startFEN);
    EXPECT_EQ(board.sideToMove, WHITE);

    std::string fen2 = FEN::to_fen(board);
    EXPECT_EQ(fen2.substr(0, startFEN.size()), startFEN);

    // Custom FEN
    std::string customFEN = "r1bqkb1r/ppp2ppp/2n1pn2/3p4/3P4/2N1PN2/PPP2PPP/8 b KQkq -";
    Board customBoard = FEN::parse(customFEN);
    std::string fen3 = FEN::to_fen(customBoard);
    EXPECT_EQ(fen3.substr(0, customFEN.size()), customFEN);

    // Test en passant
    std::string epFEN = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3";
    Board epBoard = FEN::parse(epFEN);
    EXPECT_NE(epBoard.enPassant, NO_SQUARE);
    std::string epFEN2 = FEN::to_fen(epBoard);
    EXPECT_EQ(epFEN2.substr(0, epFEN.size()), epFEN);

    // Test no castling rights
    std::string noCastle = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - -";
    Board noCastleBoard = FEN::parse(noCastle);
    EXPECT_FALSE(noCastleBoard.castling.whiteKingside);
    EXPECT_FALSE(noCastleBoard.castling.whiteQueenside);
}
