// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#pragma once

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

}
