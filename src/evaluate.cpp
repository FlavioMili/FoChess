// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#include <cstddef>
#include <cstdint>

#include "board.h"
#include "types.h"

namespace FoChess {

int bland_evaluate(const Board& board) {
  uint8_t check_ratio = board.is_in_check(static_cast<Color>(BLACK - board.sideToMove)) ? 5 : 1;
                                      //  PPP  NNN  BBB  RRR  QQQ  K
  static constexpr int piece_values[6] = {100, 320, 330, 500, 900, 0};

  int white_score = 0;
  int black_score = 0;

  for (size_t pt = 0; pt < 6; ++pt) {
    const int val = piece_values[pt];
    white_score += __builtin_popcountll(board.pieces[WHITE][pt]) * val;
    black_score += __builtin_popcountll(board.pieces[BLACK][pt]) * val;
  }

  return (white_score - black_score) * check_ratio;
}

}  // namespace Eval
