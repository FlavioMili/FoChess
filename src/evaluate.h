// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#pragma once

#include "board.h"
#include "movegen.h"
#include "types.h"


enum MoveScore : int {
  SCORE_TT_MOVE = 10000,
  SCORE_QUEEN_PROMOTION = 9000,
  SCORE_WINNING_CAPTURE = 8000,
  SCORE_EQUAL_CAPTURE = 100,
  SCORE_LOSING_CAPTURE = -100,
  SCORE_KILLER_MOVE = 50,
  SCORE_QUIET = 0,
  SCORE_CHECK = 120
};

namespace FoChess {

int bland_evaluate(const Board& board);


// MVV-LVA (Most Valuable Victim - Least Valuable Attacker)
inline int mvv_lva(Piece attacker, Piece victim) {
  static constexpr int victim_values[5] = {100, 320, 330, 500, 900};
  static constexpr int attacker_values[5] = {1, 2, 3, 4, 5};
  
  return victim_values[victim] - attacker_values[attacker];
}

// Score a single move for ordering
inline int score_move(const Move& m, const Board& board, const Move& tt_move) {
  // TT move gets highest priority
  if (m == tt_move) return SCORE_TT_MOVE;
  
  const Square from = m.from_sq();
  const Piece moved = board.piece_on(from);
  
  int score = SCORE_QUIET;
  
  if (m.type() == MoveType::PROMOTION) [[unlikely]] {
    if (m.promotion_type() == QUEEN) {
      score = SCORE_QUEEN_PROMOTION;
    } else [[unlikely]] {
      score = SCORE_QUEEN_PROMOTION - 1000;  // Under-promotions are rare
    }
  }
  
  if (board.captured_piece != NO_PIECE) {
    score = SCORE_WINNING_CAPTURE + mvv_lva(moved, board.captured_piece);
  }
  
  // Check if move gives check
  if (score == SCORE_QUIET) {
    Board tmp = board;
    tmp.makeMove(m);
    if (tmp.is_in_check(static_cast<Color>(!board.sideToMove))) {
      score = SCORE_CHECK;
    }
  }
  
  return score;
}

/// @brief Orders moves for optimal alpha-beta search performance
/// 
/// Sorts moves in descending order of expected quality to maximize alpha-beta cutoffs.
/// Better move ordering leads to more pruning and faster search.
/// 
/// Ordering priority:
/// 1. TT move (hash move) - Best move from a previous search of this position stored
///    in the transposition table. This is often the best move and causes immediate cutoffs.
/// 2. Queen promotions - Almost always good
/// 3. Captures sorted by MVV-LVA (Most Valuable Victim - Least Valuable Attacker)
///    - Capturing a queen with a pawn is better than capturing a pawn with a queen
/// 4. Quiet moves (non-captures)
/// 
/// @param moves Array of moves to be sorted in-place
/// @param n Number of valid moves in the array
/// @param board Current board position (used to identify captures and piece values)
/// @param tt_move Best move from transposition table, if available. Pass Move() if none.
///                This move will be placed first regardless of other characteristics.
inline void order_moves(std::array<Move, MAX_MOVES>& moves, size_t n, 
                       const Board& board, const Move& tt_move) {
  std::array<int, MAX_MOVES> scores;
  
  // Score all moves
  for (size_t i = 0; i < n; ++i) {
    scores[i] = score_move(moves[i], board, tt_move);
  }
  
  // Insertion sort (fast for small arrays)
  for (size_t i = 1; i < n; ++i) {
    Move move = moves[i];
    int score = scores[i];
    size_t j = i;
    
    while (j > 0 && scores[j - 1] < score) {
      moves[j] = moves[j - 1];
      scores[j] = scores[j - 1];
      --j;
    }
    
    moves[j] = move;
    scores[j] = score;
  }
}

}
