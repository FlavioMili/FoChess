// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#pragma once

#include <climits>
#include <cstdint>
#include <cstdlib>

#include "board.h"
#include "move.h"
#include "tt.h"

struct SearchResult {
  int score;
  Move move;
};

namespace FoChess {

int bland_evaluate(const Board& board);

SearchResult negamax(uint8_t depth, Board& board);
SearchResult negamax(uint8_t depth, Board& board, TranspositionTable& tt);

// Version without TT
SearchResult alpha_beta_pruning(uint8_t depth, Board& board,
                                int alpha = INT_MIN + 1,
                                int beta = INT_MAX - 1);

// Version with TT
SearchResult alpha_beta_pruning(uint8_t depth, Board& board,
                                TranspositionTable& tt, int alpha = INT_MIN + 1,
                                int beta = INT_MAX - 1);

SearchResult iterative_deepening(uint8_t max_depth, Board& board,
                                 TranspositionTable& tt);

SearchResult quiescence_search(Board& board, TranspositionTable& tt, 
                               int alpha, int beta);

inline int nodes = 0;
constexpr int MATE_SCORE = 1000000000;

}  // namespace FoChess

inline SearchResult FoChess::iterative_deepening(uint8_t max_depth,
                                                 Board& board,
                                                 TranspositionTable& tt) {
  SearchResult best_result = {0, Move()};

  for (uint8_t depth = 1; depth <= max_depth; ++depth) {
    best_result =
        alpha_beta_pruning(depth, board, tt, INT_MIN + 1, INT_MAX - 1);

    if (std::abs(best_result.score) >= MATE_SCORE - 1000) break;
  }

  return best_result;
}
