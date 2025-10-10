// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#pragma once

#include "board.h"
#include "move.h"

struct SearchResult {
  int score;
  Move move;
};

namespace FoChess {

SearchResult negamax(int depth, Board& board);

}
