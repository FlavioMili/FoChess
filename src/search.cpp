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
#include <cstdint>

#include "board.h"
#include "evaluate.h"
#include "movegen.h"
#include "tt.h"
#include "zobrist.h"

namespace FoChess {

SearchResult negamax(uint8_t depth, Board& board) {
  FoChess::nodes++;
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

SearchResult negamax(uint8_t depth, Board& board, TranspositionTable& tt) {
  nodes++;
  Bitboard key = Zobrist::generate_hash(board);

  if (TTEntry* entry = tt.probe(key)) {
    if (entry->depth >= depth) return {entry->score, entry->best_move};
  }

  if (depth == 0) return {bland_evaluate(board), Move()};

  std::array<Move, MAX_MOVES> moves;
  size_t n = MoveGen::generate_all(board, moves);
  if (n == 0)
    return board.is_in_check(board.sideToMove) ? SearchResult{INT_MIN, Move()}
                                               : SearchResult{0, Move()};

  SearchResult best = {INT_MIN, moves[0]};

  for (size_t i = 0; i < n; ++i) {
    Board tmp = board;
    tmp.makeMove(moves[i]);
    SearchResult result = negamax(depth - 1, tmp, tt);
    int score = -result.score;
    if (score > best.score) best = {score, moves[i]};
  }

  tt.store(key, best.score, best.move, depth, TT_EXACT);
  return best;
}

SearchResult alpha_beta_pruning(uint8_t depth, Board& board, int alpha,
                                int beta) {
  nodes++;
  if (depth == 0) return {bland_evaluate(board), Move()};

  std::array<Move, MAX_MOVES> moves;
  size_t n = MoveGen::generate_all(board, moves);

  if (n == 0) [[unlikely]] {
    if (board.is_in_check(board.sideToMove)) return {-MATE_SCORE, Move()};
    return {0, Move()};
  }

  SearchResult best = {INT_MIN, moves[0]};

  for (size_t i = 0; i < n; ++i) {
    Board tmp = board;
    tmp.makeMove(moves[i]);
    int score = -alpha_beta_pruning(depth - 1, tmp, -beta, -alpha).score;

    if (score > best.score) best = {score, moves[i]};
    alpha = std::max(alpha, score);

    if (alpha >= beta) break;
  }

  return best;
}

const bool should_stop_search() {
  if (g_should_stop.load()) return true;

  // Check time limit if set
  if (g_time_limit.load() > 0) {
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                       std::chrono::steady_clock::now() - g_search_start)
                       .count();

    if (elapsed >= g_time_limit.load()) return true;
  }

  return false;
}

SearchResult quiescence_search(Board& board, TranspositionTable& tt, int alpha,
                               int beta) {
  nodes++;
  if (nodes % 2048 == 0 && should_stop_search()) {
    return {bland_evaluate(board), Move()};
  }

  // Stand pat
  int stand_pat = bland_evaluate(board);
  if (stand_pat >= beta) return {stand_pat, Move()};
  if (stand_pat > alpha) alpha = stand_pat;

  std::array<Move, MAX_MOVES> moves;
  size_t n = MoveGen::generate_captures(board, moves);

  SearchResult best = {alpha, Move()};

  for (size_t i = 0; i < n; ++i) {
    Board tmp = board;
    tmp.makeMove(moves[i]);

    int score = -quiescence_search(tmp, tt, -beta, -alpha).score;

    if (score >= beta) return {score, moves[i]};
    if (score > best.score) {
      best.score = score;
      best.move = moves[i];
    }
    if (score > alpha) alpha = score;
  }

  return best;
}

SearchResult alpha_beta_pruning(uint8_t depth, Board& board,
                                TranspositionTable& tt, int alpha, int beta) {
  nodes++;
  if (nodes % 4096 == 0 && should_stop_search()) {
    return {alpha, Move()};
  }

  Bitboard key = Zobrist::generate_hash(board);
  TTEntry* tte = tt.probe(key);

  Move tt_move = Move();
  if (tte) {
    tt_move = tte->best_move;
    if (tte->depth >= depth && board.moveExists(tt_move) && board.isLegalMove(tt_move)) {
      if (tte->flag == TT_EXACT) return {tte->score, tte->best_move};
      if (tte->flag == TT_ALPHA && tte->score <= alpha)
        return {tte->score, tte->best_move};
      if (tte->flag == TT_BETA && tte->score >= beta)
        return {tte->score, tte->best_move};
    }
  }

  if (depth == 0) {
    // int score = bland_evaluate(board);
    // tt.store(key, score, Move(), 0, TT_EXACT);
    // return {score, Move()};
    return quiescence_search(board, tt, alpha, beta);
  }

  std::array<Move, MAX_MOVES> moves;
  size_t n = MoveGen::generate_all(board, moves);

  if (n == 0) {
    int score = board.is_in_check(board.sideToMove) ? -MATE_SCORE : 0;
    tt.store(key, score, Move(), depth, TT_EXACT);
    return {score, Move()};
  }

  SearchResult best = {INT_MIN+1, Move()};
  TTFlag flag = TT_ALPHA;

  for (size_t i = 0; i < n; ++i) {
    Board tmp = board;
    tmp.makeMove(moves[i]);

    if (should_stop_search()) return best.move != Move() ? best : SearchResult{alpha, tt_move};
    int score = -alpha_beta_pruning(depth - 1, tmp, tt, -beta, -alpha).score;

    if (score > best.score) {
      best.score = score;
      best.move = moves[i];

      if (score > alpha) {
        alpha = score;
        flag = TT_EXACT;
        if (score >= beta) {
          flag = TT_BETA;
          tt.store(key, best.score, best.move, depth, flag);
          break;
        }
      }
    }
  }

  tt.store(key, best.score, best.move, depth, flag);

  return best;
}

}  // namespace FoChess
