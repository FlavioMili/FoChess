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
#include <vector>
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

TEST_F(MoveGenTest, QuietPromotionTargetsAndOrder) {
    // Multiple promotions, a blocked pawn, ordinary pawns, both colors,
    // an absolute pin, and a checked king exercise both noisy-generation paths.
    const std::array<const char*, 6> positions = {
        "2n4k/P1P2P2/8/8/8/8/3P4/7K w - - 0 1",
        "7k/3p4/8/8/8/8/p1p2p2/2N4K b - - 0 1",
        "7k/8/8/8/8/8/PPPP4/7K w - - 0 1",
        "7k/pppp4/8/8/8/8/8/7K b - - 0 1",
        "7k/rP5K/8/8/8/8/8/8 w - - 0 1",
        "7k/P7/8/8/8/8/8/r6K w - - 0 1"
    };
    const std::array<size_t, 6> expected_counts = {8, 8, 0, 0, 0, 0};
    for (size_t p = 0; p < positions.size(); ++p) {
        SCOPED_TRACE(positions[p]);
        const Board board = FEN::parse(positions[p]);
        std::array<Move, MAX_MOVES> all, noisy;
        const size_t n_all = MoveGen::generate_all(board, all);
        const size_t n_noisy = MoveGen::generate_noisy(
            board, noisy, board.is_in_check(board.sideToMove));
        const auto quiet_promotions = [&](const auto& moves, size_t count) {
            std::vector<uint16_t> result;
            for (size_t i = 0; i < count; ++i)
                if (moves[i].type() == PROMOTION &&
                    board.piece_on(moves[i].to_sq()) == NO_PIECE)
                    result.push_back(moves[i].raw());
            return result;
        };
        const auto expected = quiet_promotions(all, n_all);
        EXPECT_EQ(expected.size(), expected_counts[p]);
        EXPECT_EQ(quiet_promotions(noisy, n_noisy), expected);
    }
}

TEST_F(MoveGenTest, EnPassantPrefixSafety) {
    for (const char* fen : {
        "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1",
        "rnbqkbnr/pppp1ppp/8/4p3/8/8/PPPPPPPP/RNBQKBNR w KQkq e6 0 2"
    }) {
        SCOPED_TRACE(fen);
        Board board = FEN::parse(fen);
        std::array<Move, MAX_MOVES> with_target, without_target;
        const size_t count = MoveGen::generate_all(board, with_target);
        board.enPassant = NO_SQUARE;
        const size_t other_count = MoveGen::generate_all(board, without_target);
        ASSERT_EQ(count, other_count);
        for (size_t i = 0; i < count; ++i)
            EXPECT_EQ(with_target[i], without_target[i]);
    }
    const std::array<const char*, 4> positions = {
        "7k/8/8/5pP1/8/8/8/K7 w - f6 0 1",
        "7k/8/8/r4pPK/8/8/8/8 w - f6 0 1",
        "k7/8/8/8/5Pp1/8/8/7K b - f3 0 1",
        "8/8/8/8/R4Ppk/8/8/7K b - f3 0 1"
    };
    for (size_t p = 0; p < positions.size(); ++p) {
        SCOPED_TRACE(positions[p]);
        const Board board = FEN::parse(positions[p]);
        std::array<Move, MAX_MOVES> moves;
        const size_t count = MoveGen::generate_all(board, moves);
        size_t ep_count = 0;
        for (size_t i = 0; i < count; ++i)
            if (moves[i].type() == EN_PASSANT) ++ep_count;
        EXPECT_EQ(ep_count, p % 2 == 0 ? 1U : 0U);
    }
}

TEST_F(MoveGenTest, StandardPositions) {
    TestBoardGeneration("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -", 20);
    TestBoardGeneration("8/8/8/1pP3n1/8/5k2/5N2/4K3 w - b6 0 2", 11);
    TestBoardGeneration("8/4k3/4q3/8/8/4N3/8/4K3 w - - 0 2", 5);
    TestBoardGeneration("8/8/8/8/8/1q6/8/K1k5 w - - 0 1", 0);
    TestBoardGeneration("8/4k2P/8/Pp4p1/8/8/8/R3K2R w KQ - 0 1", 25);
    TestBoardGeneration("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 b - - 0 1 ", 15);
}
