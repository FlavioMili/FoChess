#pragma once

#include <string>

#include "bitboard.h"
#include "board.h"

namespace FEN {

Board parse(const std::string& fen);
std::string to_fen(const Board& board);

}  // namespace FEN
