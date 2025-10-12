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

int bland_evaluate(const Board& board) {
  //                                      PPP  NNN  BBB  RRR  QQQ
  static constexpr int piece_values[5] = {100, 320, 330, 500, 900};

  int score = 0;

  for (size_t pt = 0; pt < 5; ++pt) {
    const int val = piece_values[pt];
    score += __builtin_popcountll(board.pieces[WHITE][pt]) * val -
             __builtin_popcountll(board.pieces[BLACK][pt]) * val;
  }

  if (board.is_in_check(board.sideToMove)) score -= 200;
  if (board.is_in_check(static_cast<Color>(!board.sideToMove))) score += 200;

  return (board.sideToMove == WHITE) ? score : -score;
}

}  // namespace FoChess
