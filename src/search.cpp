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

////////////////////////////////////////////////////////////////
// this is not the function to be used in search,
// it is kept here just for testing
///////////////////////////////////////////////////////////////
int alpha_beta_pruning(int depth, Board& board, int alpha, int beta, int ply) {
  if ((g_search_stats.node_count.load(std::memory_order_relaxed) & 2047) == 0 &&
      should_stop_search()) {
    return alpha;
  }

  if (depth == 0) return quiescence_search(board, alpha, beta);

  FoChess::g_search_stats.node_count.fetch_add(1, std::memory_order_relaxed);

  std::array<Move, MAX_MOVES> moves;
  size_t n = MoveGen::generate_all(board, moves);

  if (n == 0) [[unlikely]] {
    if (board.is_in_check(board.sideToMove)) {
      return -MATE_SCORE + ply;  // Prefer shorter mates
    }
    return 0;  // stalemate
  }

  int best = INT_MIN + 1;
  Move best_move;

  for (size_t i = 0; i < n; ++i) {
    Board tmp = board;
    tmp.makeMove(moves[i]);

    int score = -alpha_beta_pruning(depth - 1, tmp, -beta, -alpha, ply + 1);

    if (score > best) {
      best = score;
      best_move = moves[i];

      // Update global best move only at root (ply == 0)
      if (ply == 0) {
        FoChess::g_search_stats.best_move.store(best_move,
                                                std::memory_order_relaxed);
      }
    }

    alpha = std::max(alpha, score);
    if (alpha >= beta) break;  // Beta cutoff
  }

  return best;
}

int quiescence_search(Board& board, int alpha,
                      int beta) {
  if (should_stop_search()) {
    return bland_evaluate(board);
  }

  // Stand pat
  int stand_pat = bland_evaluate(board);
  if (stand_pat >= beta) return stand_pat;
  if (stand_pat > alpha) alpha = stand_pat;

  std::array<Move, MAX_MOVES> moves;
  size_t n = MoveGen::generate_captures(board, moves);

  for (size_t i = 0; i < n; ++i) {
    Board tmp = board;
    tmp.makeMove(moves[i]);

    int score = -quiescence_search(tmp, -beta, -alpha);

    if (score >= beta) return score;
    if (score > alpha) {
      alpha = score;
    }
  }

  return alpha;
}

}  // namespace FoChess
