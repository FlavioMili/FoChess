// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#pragma once
#include <array>
#include <cstddef>
#include <cstdint>

#include "types.h"

typedef uint64_t Bitboard;

namespace Bitboards {

// Positions on board
constexpr Bitboard CENTER = 0x0000181800000000ULL;
constexpr Bitboard QUEEN_SIDE = 0x0F0F0F0F0F0F0F0FULL;
constexpr Bitboard KING_SIDE  = 0xF0F0F0F0F0F0F0F0ULL;

// Files
constexpr Bitboard FILE_A = 0x0101010101010101ULL;
constexpr Bitboard FILE_B = FILE_A << 1;
constexpr Bitboard FILE_C = FILE_A << 2;
constexpr Bitboard FILE_D = FILE_A << 3;
constexpr Bitboard FILE_E = FILE_A << 4;
constexpr Bitboard FILE_F = FILE_A << 5;
constexpr Bitboard FILE_G = FILE_A << 6;
constexpr Bitboard FILE_H = FILE_A << 7;

// Ranks
constexpr Bitboard RANK_8 = 0x00000000000000FFULL;
constexpr Bitboard RANK_7 = 0x000000000000FF00ULL;
constexpr Bitboard RANK_6 = 0x0000000000FF0000ULL;
constexpr Bitboard RANK_5 = 0x00000000FF000000ULL;
constexpr Bitboard RANK_4 = 0x000000FF00000000ULL;
constexpr Bitboard RANK_3 = 0x0000FF0000000000ULL;
constexpr Bitboard RANK_2 = 0x00FF000000000000ULL;
constexpr Bitboard RANK_1 = 0xFF00000000000000ULL;

constexpr Bitboard square_bb(Square sq) noexcept {
  return Bitboard(1) << static_cast<uint8_t>(sq);
}

constexpr int popcount(Bitboard bb) noexcept {
  return __builtin_popcountll(bb);
}

constexpr uint8_t file_of(Square sq) noexcept {
  return static_cast<uint8_t>(sq % 8);
}

constexpr uint8_t rank_of(Square sq) noexcept {
  return static_cast<uint8_t>(7 - (static_cast<int>(sq) / 8));
}

// White castling
constexpr Bitboard WK_EMPTY = square_bb(F1) | square_bb(G1);
constexpr Bitboard WK_SAFE  = square_bb(E1) | square_bb(F1) | square_bb(G1);
constexpr Bitboard WQ_EMPTY = square_bb(B1) | square_bb(C1) | square_bb(D1);
constexpr Bitboard WQ_SAFE  = square_bb(E1) | square_bb(D1) | square_bb(C1);

// Black castling
constexpr Bitboard BK_EMPTY = square_bb(F8) | square_bb(G8);
constexpr Bitboard BK_SAFE  = square_bb(E8) | square_bb(F8) | square_bb(G8);
constexpr Bitboard BQ_EMPTY = square_bb(B8) | square_bb(C8) | square_bb(D8);
constexpr Bitboard BQ_SAFE  = square_bb(E8) | square_bb(D8) | square_bb(C8);


constexpr Square up(Square sq)        { return Square(sq - 8); }
constexpr Square down(Square sq)      { return Square(sq + 8); }
constexpr Square left(Square sq)      { return Square(sq - 1); }
constexpr Square right(Square sq)     { return Square(sq + 1); }
constexpr Square up_left(Square sq)   { return up(left(sq)); }
constexpr Square up_right(Square sq)  { return up(right(sq)); }
constexpr Square down_left(Square sq) { return down(left(sq)); }
constexpr Square down_right(Square sq){ return down(right(sq)); }

constexpr Bitboard bb_up(Bitboard bb)         { return bb >> 8; }
constexpr Bitboard bb_down(Bitboard bb)       { return bb << 8; }
constexpr Bitboard bb_left(Bitboard bb)       { return (bb >> 1) & ~FILE_H; }
constexpr Bitboard bb_right(Bitboard bb)      { return (bb << 1) & ~FILE_A; }
constexpr Bitboard bb_up_left(Bitboard bb)    { return bb_up(bb_left(bb)); }
constexpr Bitboard bb_up_right(Bitboard bb)   { return bb_up(bb_right(bb)); }
constexpr Bitboard bb_down_left(Bitboard bb)  { return bb_down(bb_left(bb)); }
constexpr Bitboard bb_down_right(Bitboard bb) { return bb_down(bb_right(bb)); }

/* *******************************************************************
                               PAWN MOVES
**********************************************************************/
constexpr std::array<Bitboard, 64> make_white_pawn_attacks_mask() {
  std::array<Bitboard, 64> table{};

  for (size_t sq = 0; sq < 64; ++sq) {
    Bitboard bb = 0ULL;
    int rank = rank_of(Square(sq));
    int file = file_of(Square(sq));

    // White pawn attacks: up-left, up-right
    if (file > 0 && rank < 7) bb |= Bitboards::bb_up_left(square_bb(Square(sq)));
    if (file < 7 && rank < 7) bb |= Bitboards::bb_up_right(square_bb(Square(sq)));

    table[sq] = bb;
  }
  return table;
}

constexpr std::array<Bitboard, 64> make_black_pawn_attacks_mask() {
  std::array<Bitboard, 64> table{};

  for (size_t sq = 0; sq < 64; ++sq) {
    Bitboard bb = 0ULL;
    int rank = rank_of(Square(sq));
    int file = file_of(Square(sq));

    // Black pawn attacks: down-left, down-right
    if (file > 0 && rank > 0) bb |= Bitboards::bb_down_left(square_bb(Square(sq)));
    if (file < 7 && rank > 0) bb |= Bitboards::bb_down_right(square_bb(Square(sq)));

    table[sq] = bb;
  }

  return table;
}

constexpr std::array<Bitboard, 64> make_white_pawn_moves_mask() {
  std::array<Bitboard, 64> table{};

  for (size_t sq = 0; sq < 64; ++sq) {
    Bitboard bb = 0ULL;
    uint8_t rank = rank_of(Square(sq));

    auto up = Bitboards::bb_up(Bitboards::square_bb(Square(sq)));
    if (rank < 7) bb |= up;
    if (rank == 1) bb |= Bitboards::bb_up(up);

    table[sq] = bb;
  }

  return table;
}

constexpr std::array<Bitboard, 64> make_black_pawn_moves_mask() {
  std::array<Bitboard, 64> table{};

  for (size_t sq = 0; sq < 64; ++sq) {
    Bitboard bb = 0ULL;
    uint8_t rank = rank_of(Square(sq));

    auto down = Bitboards::bb_down(Bitboards::square_bb(Square(sq)));
    if (rank > 0) bb |= down;
    if (rank == 6) bb |= Bitboards::bb_down(down);

    table[sq] = bb;
  }

  return table;
}

constexpr auto black_pawn_moves_mask = make_black_pawn_moves_mask();
constexpr auto white_pawn_moves_mask = make_white_pawn_moves_mask();
constexpr auto white_pawn_attacks_mask = make_white_pawn_attacks_mask();
constexpr auto black_pawn_attacks_mask = make_black_pawn_attacks_mask();

inline Bitboard pawn_moves(Square sq, Color c, Bitboard empty) {
  const auto& mask = (c == WHITE) ? white_pawn_moves_mask : black_pawn_moves_mask;
  Bitboard moves = mask[sq];

  // Remove blocked squares
  Bitboard one_step = (c == WHITE) ? (Bitboards::bb_up(Bitboards::square_bb(sq)) & empty)
    : (Bitboards::bb_down(Bitboards::square_bb(sq)) & empty);

  // If the single-step square is blocked, remove all moves
  if (!one_step) moves &= ~mask[sq];
  else {
    // If double push exists in the mask but the intermediate square is blocked, remove it
    Bitboard double_push = moves & ~one_step;
    moves &= ~((double_push) & ~(Bitboards::bb_up(one_step) | Bitboards::bb_down(one_step)));
  }

  return moves & empty;
}

inline Bitboard pawn_attacks_mask(Square sq, Color c) {
  return (c == WHITE ? white_pawn_attacks_mask[sq] : black_pawn_attacks_mask[sq]);
}

inline Bitboard pawn_attacks(Square sq, Color c, Bitboard enemies) {
  return pawn_attacks_mask(sq, c) & enemies;
}

/* *******************************************************************
                               KNIGHT MOVES
**********************************************************************/

constexpr std::array<Bitboard, 64> make_knight_attacks() {
  std::array<Bitboard, 64> table{};

  for (size_t sq = 0; sq < 64; ++sq) {
    Bitboard bb = Bitboards::square_bb(Square(sq));

    Bitboard attacks = 0;
    attacks |= Bitboards::bb_up(Bitboards::bb_up_left(bb));        // up-up-left
    attacks |= Bitboards::bb_up(Bitboards::bb_up_right(bb));       // up-up-right
    attacks |= Bitboards::bb_down(Bitboards::bb_down_left(bb));    // down-down-left
    attacks |= Bitboards::bb_down(Bitboards::bb_down_right(bb));   // down-down-right
    attacks |= Bitboards::bb_left(Bitboards::bb_up_left(bb));      // left-left-up
    attacks |= Bitboards::bb_left(Bitboards::bb_down_left(bb));    // left-left-down
    attacks |= Bitboards::bb_right(Bitboards::bb_up_right(bb));    // right-right-up
    attacks |= Bitboards::bb_right(Bitboards::bb_down_right(bb));  // right-right-down

    table[sq] = attacks;
  }

  return table;
}

constexpr auto knight_attacks_mask = make_knight_attacks();

inline Bitboard knight_attacks(Square sq) {
  return knight_attacks_mask[sq];
}

inline Bitboard knight_moves(Square sq, Bitboard friendly) {
  return knight_attacks_mask[sq] & ~friendly;
}

/*********************************************************************
                               KING MOVES
**********************************************************************/

// Compile-time king attacks table
constexpr std::array<Bitboard, 64> make_king_attacks_mask() {
  std::array<Bitboard, 64> table{};
  for (size_t sq = 0; sq < 64; ++sq) {
    Bitboard bb = Bitboards::square_bb(Square(sq));
    table[sq] = Bitboards::bb_up(bb) | Bitboards::bb_down(bb) | Bitboards::bb_left(bb) |
                Bitboards::bb_right(bb) | Bitboards::bb_up_left(bb) | Bitboards::bb_up_right(bb) |
                Bitboards::bb_down_left(bb) | Bitboards::bb_down_right(bb);
  }
  return table;
}

constexpr auto king_attacks_mask = make_king_attacks_mask();

// Runtime helpers
inline Bitboard king_attacks(Square sq) {
  return king_attacks_mask[sq];
}

inline Bitboard king_moves(Square sq, Bitboard friendly) {
  return king_attacks_mask[sq] & ~friendly;
}

/*********************************************************************
                       MAGIC MOVES ARE IN magic.h
**********************************************************************/

}  // namespace Bitboards
