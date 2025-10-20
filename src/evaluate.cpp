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

static constexpr int piece_values[5] = {180, 550, 630, 900, 1340};
static constexpr int pawn_table[64] = {
    0,  0,  0,  0,  0,  0,  0,  0,
    5, 10, 10,20,20, 10, 10,  5,
    5, 5,10,  0,  0,10, 5,  5,
    0,  0,  0, 20, 20,  0,  0,  0,
    5,  5, 10,25,25, 10,  5,  5,
   10, 10, 20,20,20, 20, 10, 10,
   50, 50, 50,50,50, 50, 50, 50,
    0,  0,  0,  0,  0,  0,  0,  0
};

static constexpr int knight_table[64] = {
   -50,-40,-30,-30,-30,-30,-40,-50,
   -40,-20,  0,  0,  0,  0,-20,-40,
   -30,  0, 10,15,15,10,  0,-30,
   -30,  5, 15,20,20,15,  5,-30,
   -30,  0, 15,20,20,15,  0,-30,
   -30,  5, 10,15,15,10,  5,-30,
   -40,-20,  0,  5,  5,  0,-20,-40,
   -50,-40,-30,-30,-30,-30,-40,-50
};

static constexpr int bishop_table[64] = {
   -20,-10,-10,-10,-10,-10,-10,-20,
   -10,  0,  0,  0,  0,  0,  0,-10,
   -10,  0,  5,10,10, 5,  0,-10,
   -10,  5,  5,10,10, 5,  5,-10,
   -10,  0, 10,10,10,10,  0,-10,
   -10, 10, 10,10,10,10, 10,-10,
   -10,  9,  0,  0,  0,  0,  9,-10,
   -20,-10,-10,-10,-10,-10,-10,-20
};

static constexpr int rook_table[64] = {
     0,  0,  0,  0,  0,  0,  0,  0,
     5, 10, 10, 10, 10, 10, 10,  5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
     0, 10, 10, 10, 10, 10, 10,  0,
     0,  0,  0,  5,  5,  0,  0,  0
};

static constexpr int queen_table[64] = {
   -20,-10,-10, -5, -5,-10,-10,-20,
   -10,  0,  0,  0,  0,  0,  0,-10,
   -10,  0,  2,  2,  2,  2,  0,-10,
    -5,  0,  3,  4,  4,  3,  0, -5,
     0,  0,  3,  5,  5,  3,  0, -5,
   -10,  5,  5,  5,  5,  5,  0,-10,
   -10,  0,  5,  0,  0,  0,  0,-10,
   -20,-10,-10, -5, -5,-10,-10,-20
};

constexpr const int* pst_tables[5] = {
    pawn_table, knight_table, bishop_table, rook_table, queen_table
};

// -----------------------------------------------------------------------------
//  Evaluation function
// -----------------------------------------------------------------------------
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
