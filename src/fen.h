// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#pragma once

#include <string>

#include "board.h"

namespace FEN {

Board parse(const std::string& fen);
std::string to_fen(const Board& board);

}  // namespace FEN
