#include <cstddef>

#include "board.h"
#include "types.h"

namespace Eval {

int bland_evaluate(const Board& board) {
                                      //  PPP  NNN  BBB  RRR  QQQ  K
  static constexpr int piece_values[6] = {100, 320, 330, 500, 900, 0};

  int white_score = 0;
  int black_score = 0;

  for (size_t pt = 0; pt < 6; ++pt) {
    const int val = piece_values[pt];
    white_score += __builtin_popcountll(board.pieces[WHITE][pt]) * val;
    black_score += __builtin_popcountll(board.pieces[BLACK][pt]) * val;
  }

  return (white_score - black_score);
}

}  // namespace Eval
