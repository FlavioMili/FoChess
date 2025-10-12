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
  Bitboard key = Zobrist::generate_hash(board);

  if (TTEntry* entry = tt.probe(key)) {
    if (entry->depth >= depth) return {entry->score, entry->move};
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

  tt.store(key, best.score, best.move, depth, TTFlags(1, 0, 0));
  return best;
}

}  // namespace FoChess
