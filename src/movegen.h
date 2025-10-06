// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#pragma once

#include <sys/types.h>

#include <array>
#include <cstdint>

#include "board.h"
#include "move.h"

constexpr uint8_t MAX_MOVES = 218;

namespace MoveGen {

// Generate all pseudo-legal moves for the side to move
size_t generate_all(const Board& board, std::array<Move, MAX_MOVES>& moves);

}  // namespace MoveGen
