// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#include <cstddef>
#include "board.h"
#include "types.h"
  
namespace FoChess {

static constexpr std::array<int, 5> piece_values = {180, 550, 630, 900, 1340};

static constexpr std::array<int, 64> pawn_table = {
    990,  990,  990,  990,  990,  990,  990,  990,
    5, 10, 10,20,20, 10, 10,  5,
    5, 5,10,  0,  0,10, 5,  5,
    0,  0,  0, 20, 20,  0,  0,  0,
    5,  5, 10,25,25, 10,  5,  5,
    10, 20, 20,20,20, 20, 20, 10,
    30, 30, 20,19,19, 20, 30, 30,
    0,  0,  0,  0,  0,  0,  0,  0
};

static constexpr std::array<int, 64> knight_table = {
   -50,-40,-30,-30,-30,-30,-40,-50,
   -40,-20,  0,  0,  0,  0,-20,-40,
   -30,  0, 10,15,15,10,  0,-30,
   -30,  5, 15,20,20,15,  5,-30,
   -30,  0, 15,20,20,15,  0,-30,
   -30,  5, 10,15,15,10,  5,-30,
   -40,-20,  0,  5,  5,  0,-20,-40,
   -50,-40,-30,-30,-30,-30,-40,-50
};

static constexpr std::array<int, 64> bishop_table = {
   -20,-10,-10,-10,-10,-10,-10,-20,
   -10,  0,  0,  0,  0,  0,  0,-10,
   -10,  0,  5,10,10, 5,  0,-10,
   -10,  5,  5,10,10, 5,  5,-10,
   -10,  0, 10,10,10,10,  0,-10,
   -10, 10, 10,10,10,10, 10,-10,
   -10,  9,  0,  0,  0,  0,  9,-10,
   -20,-10,-10,-10,-10,-10,-10,-20
};

static constexpr std::array<int, 64> rook_table = {
     0,  1,  1,  1,  1,  1,  1,  0,
     5, 10, 10, 10, 10, 10, 10,  5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
     0, 10, 10, 10, 10, 10, 10,  0,
     0,  0,  0,  5,  5,  0,  0,  0
};

static constexpr std::array<int, 64> queen_table = {
   -20,-10,-10, -5, -5,-10,-10,-20,
   -10,  1,  2,  1,  1,  2,  1,-10,
   -10,  0,  2,  2,  2,  2,  1,-10,
    -5,  0,  3,  4,  4,  3,  1, -5,
     0,  0,  3,  5,  5,  3,  1, -5,
   -10,  5,  5,  5,  5,  5,  1,-10,
   -10,  0,  5,  0,  0,  0,  0,-10,
   -20,-10,-10, -5, -5,-10,-10,-20
};

constexpr std::array<const int*, 5> pst_tables = {
    pawn_table.data(), knight_table.data(), bishop_table.data(),
    rook_table.data(), queen_table.data()};

int bland_evaluate(const Board& board) {
  int score = 0;

  for (size_t pt = 0; pt < 5; ++pt) {
    const int val = piece_values[pt];
    const int* pst = pst_tables[pt];

    Bitboard bbW = board.pieces[WHITE][pt];
    while (bbW) {
      const int sq = std::countr_zero(bbW);
      bbW &= bbW - 1;
      score += val + pst[sq];
    }

    Bitboard bbB = board.pieces[BLACK][pt];
    while (bbB) {
      const int sq = std::countr_zero(bbB);
      bbB &= bbB - 1;
      // mirror vertically for black’s perspective
      score -= val + pst[sq ^ 56];
    }
  }

  return (board.sideToMove == WHITE) ? score : -score;
}

}  // namespace FoChess
