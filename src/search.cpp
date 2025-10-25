// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#include "search.h"

#include <array>
#include <cstddef>

#include "board.h"
#include "evaluate.h"
#include "movegen.h"
#include "tt.h"

namespace FoChess {

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
    if (board.is_in_check(board.sideToMove)) return -MATE_SCORE;
    return 0;
  }

  int best = -INF_SCORE;
  Move best_move = moves[0];

  for (size_t i = 0; i < n; i++) {
    Board tmp = board;
    tmp.makeMove(moves[i]);
    int score = -alpha_beta_pruning(depth - 1, tmp, -beta, -alpha, ply + 1);

    if (score > best) {
      best = score;
      best_move = moves[i];
    }

    alpha = std::max(alpha, score);
    if (alpha >= beta) break;
  }

  if (ply == 0) {
    g_search_stats.best_move.store(best_move, std::memory_order_relaxed);
    g_search_stats.best_root_score.store(best, std::memory_order_relaxed);
  }

  return best;
}

int quiescence_search(Board& board, int alpha, int beta) {
  if ((g_search_stats.node_count.load(std::memory_order_relaxed) & 2047) == 0 &&
      should_stop_search())
    return bland_evaluate(board);

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
    if (score > alpha) alpha = score;
  }

  return alpha;
}

int alpha_beta_pruning(int depth, Board& board, TranspositionTable& tt,
                       int alpha, int beta, int ply) {
  if ((g_search_stats.node_count.load(std::memory_order_relaxed) & 2047) == 0 &&
      should_stop_search())
    return alpha;

  const uint64_t hash_key = board.hash;
  TTEntry* tte = tt.probe(hash_key);

  Move tt_move = Move();
  if (tte) {
    tt_move = tte->best_move;
    if (tte->depth >= depth && board.moveExists(tt_move) &&
        board.isLegalMove(tt_move)) {
      if (tte->flag == TT_EXACT) return tte->score;
      if (tte->flag == TT_ALPHA && tte->score <= alpha) return tte->score;
      if (tte->flag == TT_BETA && tte->score >= beta) return tte->score;
    }
  }

  if (depth == 0) return quiescence_search(board, tt, alpha, beta);

  FoChess::g_search_stats.node_count.fetch_add(1, std::memory_order_relaxed);

  std::array<Move, MAX_MOVES> moves;
  size_t n = MoveGen::generate_all(board, moves);
  if (n == 0)
    return (board.is_in_check(board.sideToMove)) ? -MATE_SCORE + ply : 0;

  if (board.moveExists(tt_move) && board.isLegalMove(tt_move)) {
    auto it = std::find(moves.begin(), moves.begin() + n, tt_move);
    if (it != moves.begin() + n) std::iter_swap(moves.begin(), it);
  }

  int best = -INF_SCORE;
  Move best_move;

  TTFlag flag = TT_ALPHA;

  for (size_t i = 0; i < n; ++i) {
    Board tmp = board;
    tmp.makeMove(moves[i]);

    int score = -alpha_beta_pruning(depth - 1, tmp, tt, -beta, -alpha, ply + 1);

    if (score > best) {
      best = score;
      best_move = moves[i];

      if (ply == 0)
        g_search_stats.best_move.store(best_move, std::memory_order_relaxed);
      if (score > alpha) {
        alpha = score;
        flag = TT_EXACT;
        if (score >= beta) {
          flag = TT_BETA;
          tt.store(hash_key, best, best_move, static_cast<uint8_t>(depth), flag);
          break;
        }
      }
    }
  }

  tt.store(hash_key, best, best_move, static_cast<uint8_t>(depth), flag);
  return best;
}

int quiescence_search(Board& board, TranspositionTable& tt, int alpha,
                      int beta) {
  if ((g_search_stats.node_count.load(std::memory_order_relaxed) & 2047) == 0 &&
      should_stop_search()) {
    return bland_evaluate(board);
  }

  FoChess::g_search_stats.node_count.fetch_add(1, std::memory_order_relaxed);

  // Stand pat
  int stand_pat = bland_evaluate(board);
  if (stand_pat >= beta) return beta;
  if (stand_pat > alpha) alpha = stand_pat;

  std::array<Move, MAX_MOVES> moves;
  size_t n = MoveGen::generate_captures(board, moves);

  for (size_t i = 0; i < n; ++i) {
    Board tmp = board;
    tmp.makeMove(moves[i]);
    int score = -quiescence_search(tmp, tt, -beta, -alpha);

    if (score >= beta) return beta;
    if (score > alpha) alpha = score;
  }

  return alpha;
}

}  // namespace FoChess
