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
constexpr Bitboard QUEEN_SIDE = 0x0F0F0F0F0F0F0F0FULL;
constexpr Bitboard KING_SIDE = 0xF0F0F0F0F0F0F0F0ULL;

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
  return static_cast<uint8_t>(7 - (static_cast<int>(sq) / 8));
}

inline Square up(Square sq) {
  return Square(int(sq) - 8);
}
inline Square down(Square sq) {
  return Square(int(sq) + 8);
}
inline Square left(Square sq) {
  return Square(int(sq) - 1);
}
inline Square right(Square sq) {
  return Square(int(sq) + 1);
}
inline Square up_left(Square sq) {
  return up(left(sq));
}
inline Square up_right(Square sq) {
  return up(right(sq));
}
inline Square down_left(Square sq) {
  return down(left(sq));
}
inline Square down_right(Square sq) {
  return down(right(sq));
}
inline Bitboard bb_up(Bitboard bb) {
  return bb >> 8;
}
inline Bitboard bb_down(Bitboard bb) {
  return bb << 8;
}
inline Bitboard bb_left(Bitboard bb) {
  return (bb >> 1) & ~FILE_H;
}
inline Bitboard bb_right(Bitboard bb) {
  return (bb << 1) & ~FILE_A;
}
inline Bitboard bb_up_left(Bitboard bb) {
  return bb_up(bb_left(bb));
}
inline Bitboard bb_up_right(Bitboard bb) {
  return bb_up(bb_right(bb));
}
inline Bitboard bb_down_left(Bitboard bb) {
  return bb_down(bb_left(bb));
}
inline Bitboard bb_down_right(Bitboard bb) {
  return bb_down(bb_right(bb));
}

inline Bitboard pawn_attacks_mask(Square sq, Color c) {
  Bitboard bb = square_bb(sq);
  if (c == WHITE)
    return bb_up_left(bb) | bb_up_right(bb);
  else
    return bb_down_left(bb) | bb_down_right(bb);
}

inline Bitboard pawn_attacks(Square sq, Color c, Bitboard enemies) {
  // Mask the generic attack mask with the enemy bitboard so captures only appear
  return pawn_attacks_mask(sq, c) & enemies;
}

inline Bitboard pawn_moves(Square sq, Color c, Bitboard empty, Bitboard friendly) {
  Bitboard moves = 0;
  Bitboard bb = square_bb(sq);
  Bitboard enemies = (~empty) & ~friendly;

  if (c == WHITE) {
    Bitboard one = bb_up(bb) & empty;
    if (one) {
      moves |= one;
      if (rank_of(sq) == 1) moves |= bb_up(one) & empty;
    }
    moves |= pawn_attacks(sq, WHITE, enemies);
  } else {
    Bitboard one = bb_down(bb) & empty;
    if (one) {
      moves |= one;
      if (rank_of(sq) == 6) moves |= bb_down(one) & empty;
    }
    moves |= pawn_attacks(sq, BLACK, enemies);
  }
  return moves;
}

inline Bitboard knight_attacks(Square sq) {
  Bitboard bb = square_bb(sq);
  Bitboard l1 = (bb >> 1) & ~FILE_H;
  Bitboard l2 = (bb >> 2) & ~(FILE_H | (FILE_H >> 1));
  Bitboard r1 = (bb << 1) & ~FILE_A;
  Bitboard r2 = (bb << 2) & ~(FILE_A | (FILE_A << 1));

  return (l2 << 8) | (l2 >> 8) | (r2 << 8) | (r2 >> 8) | (l1 << 16) | (l1 >> 16) | (r1 << 16) |
         (r1 >> 16);
}

inline Bitboard knight_moves(Square sq, Bitboard friendly) {
  return knight_attacks(sq) & ~friendly;
}

inline Bitboard king_attacks(Square sq) {
  Bitboard bb = square_bb(sq);
  return bb_up(bb) | bb_down(bb) | bb_left(bb) | bb_right(bb) | bb_up_left(bb) | bb_up_right(bb) |
         bb_down_left(bb) | bb_down_right(bb);
}

inline Bitboard king_moves(Square sq, Bitboard friendly) {
  return king_attacks(sq) & ~friendly;
}

inline Bitboard rook_attacks(Square sq, Bitboard occ) {
  Bitboard attacks = 0;
  Bitboard bb = square_bb(sq);

  for (Bitboard t = bb_up(bb); t; t = bb_up(t)) {
    attacks |= t;
    if (t & occ) break;
  }
  for (Bitboard t = bb_down(bb); t; t = bb_down(t)) {
    attacks |= t;
    if (t & occ) break;
  }
  for (Bitboard t = bb_left(bb); t; t = bb_left(t)) {
    attacks |= t;
    if (t & occ) break;
  }
  for (Bitboard t = bb_right(bb); t; t = bb_right(t)) {
    attacks |= t;
    if (t & occ) break;
  }

  return attacks;
}

inline Bitboard bishop_attacks(Square sq, Bitboard occ) {
  Bitboard attacks = 0;
  Bitboard bb = square_bb(sq);

  for (Bitboard t = bb_up_left(bb); t; t = bb_up_left(t)) {
    attacks |= t;
    if (t & occ) break;
  }
  for (Bitboard t = bb_up_right(bb); t; t = bb_up_right(t)) {
    attacks |= t;
    if (t & occ) break;
  }
  for (Bitboard t = bb_down_left(bb); t; t = bb_down_left(t)) {
    attacks |= t;
    if (t & occ) break;
  }
  for (Bitboard t = bb_down_right(bb); t; t = bb_down_right(t)) {
    attacks |= t;
    if (t & occ) break;
  }

  return attacks;
}

inline Bitboard queen_attacks(Square sq, Bitboard occ) {
  return rook_attacks(sq, occ) | bishop_attacks(sq, occ);
}

inline Bitboard rook_moves(Square sq, Bitboard occ, Bitboard friendly) {
  return rook_attacks(sq, occ) & ~friendly;
}

inline Bitboard bishop_moves(Square sq, Bitboard occ, Bitboard friendly) {
  return bishop_attacks(sq, occ) & ~friendly;
}

inline Bitboard queen_moves(Square sq, Bitboard occ, Bitboard friendly) {
  return queen_attacks(sq, occ) & ~friendly;
}

}  // namespace Bitboards
