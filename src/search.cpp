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
#include "types.h"
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

SearchResult alpha_beta_pruning(uint8_t depth, Board& board, int alpha, int beta) {
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
    uint8_t raiseDepth = depth + (tmp.captured_piece != NO_PIECE);
    int score = -alpha_beta_pruning(raiseDepth - 1, tmp, -beta, -alpha).score;

    if (score > best.score) best = {score, moves[i]};
    alpha = std::max(alpha, score);

    if (alpha >= beta) break;
  }

  return best;
}

SearchResult alpha_beta_pruning(uint8_t depth, Board& board,
                                TranspositionTable& tt, int alpha, int beta) {
  nodes++;

  Bitboard key = Zobrist::generate_hash(board);
  TTEntry* tte = tt.probe(key);

  if (tte && tte->depth >= depth) {
    if (tte->flag == TT_EXACT) return {tte->score, tte->best_move};
    if (tte->flag == TT_ALPHA && tte->score <= alpha) return {alpha, tte->best_move};
    if (tte->flag == TT_BETA && tte->score >= beta) return {beta, tte->best_move};
  }

  if (depth == 0) {
    int score = bland_evaluate(board);
    tt.store(key, score, Move(), 0, TT_EXACT);
    return {score, Move()};
  }

  std::array<Move, MAX_MOVES> moves;
  size_t n = MoveGen::generate_all(board, moves);
  if (n == 0) {
    int score = board.is_in_check(board.sideToMove) ? -MATE_SCORE : 0;
    tt.store(key, score, Move(), depth, TT_EXACT);
    return {score, Move()};
  }

  SearchResult best = {alpha, Move()};
  TTFlag flag = TT_ALPHA;

  for (size_t i = 0; i < n; ++i) {
    Board tmp = board;
    tmp.makeMove(moves[i]);

    int score = -alpha_beta_pruning(depth - 1, tmp, tt, -beta, -alpha).score;

    if (score > best.score) {
      best.score = score;
      best.move = moves[i];

      if (score > alpha) {
        alpha = score;
        flag = TT_EXACT;
        if (score >= beta) {
          flag = TT_BETA;
          break;  // beta cutoff
        }
      }
    }
  }

  int storeScore = (flag == TT_BETA) ? beta : alpha;
  tt.store(key, storeScore, best.move, depth, flag);

  return best;
}

}
