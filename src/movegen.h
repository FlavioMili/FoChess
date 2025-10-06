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
#include "types.h"

#include "move.h"
#include <vector>

constexpr uint8_t MAX_MOVES = 218;

namespace MoveGen {

// Generate all pseudo-legal moves for the side to move
size_t generate_all(const Board& board, std::array<Move, MAX_MOVES>& moves);

// Specialized generators for debugging or profiling
void generate_pawn_moves(const Board& board, Color c, std::vector<Move>& moves);
void generate_knight_moves(const Board& board, Color c, std::vector<Move>& moves);
void generate_bishop_moves(const Board& board, Color c, std::vector<Move>& moves);
void generate_rook_moves(const Board& board, Color c, std::vector<Move>& moves);
void generate_queen_moves(const Board& board, Color c, std::vector<Move>& moves);
void generate_king_moves(const Board& board, Color c, std::vector<Move>& moves);

}  // namespace MoveGen
