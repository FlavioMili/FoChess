// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#pragma once
#include <cstdint>

#include "types.h"

typedef uint64_t Bitboard;

namespace Bitboards {

// Positions on board
constexpr Bitboard CENTER = 0x0000001818000000ULL;
constexpr Bitboard KING_SIDE = 0x00000000000000FEULL;
constexpr Bitboard QUEEN_SIDE = 0x00000000000000FFULL;

// Files
constexpr Bitboard FILE_A = 0x0101010101010101ULL;
constexpr Bitboard FILE_B = 0x0202020202020202ULL;
constexpr Bitboard FILE_C = 0x0404040404040404ULL;
constexpr Bitboard FILE_D = 0x0808080808080808ULL;
constexpr Bitboard FILE_E = 0x1010101010101010ULL;
constexpr Bitboard FILE_F = 0x2020202020202020ULL;
constexpr Bitboard FILE_G = 0x4040404040404040ULL;
constexpr Bitboard FILE_H = 0x8080808080808080ULL;

// Ranks
constexpr Bitboard RANK_1 = 0x00000000000000FFULL;
constexpr Bitboard RANK_2 = 0x000000000000FF00ULL;
constexpr Bitboard RANK_3 = 0x0000000000FF0000ULL;
constexpr Bitboard RANK_4 = 0x00000000FF000000ULL;
constexpr Bitboard RANK_5 = 0x000000FF00000000ULL;
constexpr Bitboard RANK_6 = 0x0000FF0000000000ULL;
constexpr Bitboard RANK_7 = 0x00FF000000000000ULL;
constexpr Bitboard RANK_8 = 0xFF00000000000000ULL;

inline constexpr Bitboard square_bb(Square sq) noexcept {
  return Bitboard(1) << static_cast<int>(sq);
}

inline constexpr uint8_t file_of(Square sq) noexcept {
  return static_cast<uint8_t>(sq % 8);
}

inline constexpr uint8_t rank_of(Square sq) noexcept {
  return static_cast<uint8_t>(sq / 8);
}

// Directions
Square up(Square sq);
Square down(Square sq);
Square left(Square sq);
Square right(Square sq);
Square up_left(Square sq);
Square up_right(Square sq);
Square down_left(Square sq);
Square down_right(Square sq);

// Bitboard shifting
Bitboard bb_up(Bitboard bb);
Bitboard bb_down(Bitboard bb);
Bitboard bb_left(Bitboard bb);
Bitboard bb_right(Bitboard bb);
Bitboard bb_up_left(Bitboard bb);
Bitboard bb_up_right(Bitboard bb);
Bitboard bb_down_left(Bitboard bb);
Bitboard bb_down_right(Bitboard bb);

// Pieces methods
Bitboard pawn_attacks(Square sq, Color c);
Bitboard pawn_moves(Square sq, Color c, Bitboard empty, Bitboard friendly);

Bitboard knight_attacks(Square sq);
Bitboard knight_moves(Square sq, Bitboard friendly);

Bitboard king_attacks(Square sq);
Bitboard king_moves(Square sq, Bitboard friendly);

Bitboard rook_attacks(Square sq, Bitboard occupancy);
Bitboard rook_moves(Square sq, Bitboard occupancy, Bitboard friendly);

Bitboard bishop_attacks(Square sq, Bitboard occupancy);
Bitboard bishop_moves(Square sq, Bitboard occupancy, Bitboard friendly);

Bitboard queen_attacks(Square sq, Bitboard occupancy);
Bitboard queen_moves(Square sq, Bitboard occupancy, Bitboard friendly);

}  // namespace Bitboards
