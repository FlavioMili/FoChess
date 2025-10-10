// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#include "search.h"

#include <array>
#include <climits>
#include <cstddef>
#include "board.h"
#include "evaluate.h"
#include "movegen.h"

namespace FoChess {

SearchResult negamax(int depth, Board& board) {
  if (depth == 0) {
    return {bland_evaluate(board), Move()};
  }

  std::array<Move, MAX_MOVES> moves;
  size_t n = MoveGen::generate_all(board, moves);

  if (n == 0) {
    if (board.is_in_check(board.sideToMove)) return {INT_MIN, Move()};
    return {0, Move()};
  }

  SearchResult best = {INT_MIN, moves[0]};

  for (size_t i = 0; i < n; ++i) {
    Board tmp = board;
    tmp.makeMove(moves[i]);

    SearchResult result = negamax(depth - 1, tmp);
    int score = -result.score;

    if (score > best.score) {
      best.score = score;
      best.move = moves[i];
    }
  }

  return best;
}

}
