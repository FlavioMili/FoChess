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
#include "zobrist.h"

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

  int best = INT_MIN + 1;
  Move best_local = moves[0];

  for (size_t i = 0; i < n; ++i) {
    Board tmp = board;
    tmp.makeMove(moves[i]);

    int score = -alpha_beta_pruning(depth - 1, tmp, -beta, -alpha, ply + 1);

    if (alpha >= beta) break;

    if (score > best) {
      best = score;
      if (ply == 0)
        g_search_stats.best_move.store(best_local, std::memory_order_relaxed);
    }

    alpha = std::max(alpha, score);
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
      should_stop_search()) {
    return alpha;
  }

  uint64_t hash_key = Zobrist::generate_hash(board);

  // Probe transposition table
  TTEntry* tt_entry = tt.probe(hash_key);
  Move tt_move;

  if (tt_entry && tt_entry->depth >= depth) {
    if (tt_entry->flag == TT_EXACT) {
      return tt_entry->score;
    } else if (tt_entry->flag == TT_ALPHA && tt_entry->score <= alpha) {
      return alpha;
    } else if (tt_entry->flag == TT_BETA && tt_entry->score >= beta) {
      return beta;
    }
  }

  // Get TT move for move ordering
  if (tt_entry) {
    tt_move = tt_entry->best_move;
  }

  if (depth == 0) return quiescence_search(board, tt, alpha, beta);

  FoChess::g_search_stats.node_count.fetch_add(1, std::memory_order_relaxed);

  std::array<Move, MAX_MOVES> moves;
  size_t n = MoveGen::generate_all(board, moves);

  if (n == 0) [[unlikely]] {
    if (board.is_in_check(board.sideToMove)) return -MATE_SCORE + ply;
    return 0;
  }

  int best = INT_MIN + 1;
  Move best_move = moves[0];
  TTFlag flag = TT_ALPHA;  // Assume fail-low

  for (size_t i = 0; i < n; ++i) {
    Board tmp = board;
    tmp.makeMove(moves[i]);
    int score = -alpha_beta_pruning(depth - 1, tmp, tt, -beta, -alpha, ply + 1);

    if (alpha >= beta) {
      flag = TT_BETA;  // Beta cutoff
      break;
    }
    if (score > best) {
      best = score;
      best_move = moves[i];

      if (ply == 0) {
        g_search_stats.best_move.store(best_move, std::memory_order_relaxed);
      }
    }

    alpha = std::max(alpha, score);
  }

  // If we improved alpha, it's an exact score
  if (best > alpha) {
    flag = TT_EXACT;
  }

  // Store in transposition table
  tt.store(hash_key, best, best_move, depth, flag);

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
