// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#pragma once

#include <climits>
#include <cstdint>

#include "board.h"
#include "move.h"
#include "tt.h"

struct SearchResult {
  int score;
  Move move;
};

namespace FoChess {

SearchResult negamax(uint8_t depth, Board& board);
SearchResult negamax(uint8_t depth, Board& board, TranspositionTable& tt);

// Version without TT
SearchResult alpha_beta_pruning(uint8_t depth, Board& board, int alpha = INT_MIN + 1,
                                int beta = INT_MAX);

// Version with TT
SearchResult alpha_beta_pruning(uint8_t depth, Board& board, TranspositionTable& tt,
                                int alpha = INT_MIN + 1, int beta = INT_MAX);


inline int nodes = 0;

}  // namespace FoChess
