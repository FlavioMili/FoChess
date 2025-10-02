// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#include "bitboard.h"

#include "types.h"

namespace Bitboards {

inline Square up(Square sq) {
  return Square(int(sq) + 8);
}
inline Square down(Square sq) {
  return Square(int(sq) - 8);
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
  return bb << 8;
}
inline Bitboard bb_down(Bitboard bb) {
  return bb >> 8;
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


inline Bitboard pawn_attacks(Square sq, Color c) {
  Bitboard bb = square_bb(sq);
  if (c == WHITE)
    return bb_up_left(bb) | bb_up_right(bb);
  else
    return bb_down_left(bb) | bb_down_right(bb);
}

inline Bitboard pawn_moves(Square sq, Color c, Bitboard empty, Bitboard friendly) {
  Bitboard moves = 0;
  Bitboard bb = square_bb(sq);

  if (c == WHITE) {
    Bitboard one = bb_up(bb) & empty;
    if (one) {
      moves |= one;
      // double push only from rank 2
      if (rank_of(sq) == 1)  
        moves |= bb_up(one) & empty;
    }
    moves |= pawn_attacks(sq, WHITE) & ~friendly;
  } else {
    Bitboard one = bb_down(bb) & empty;
    if (one) {
      moves |= one;
      // double push only from rank 7
      if (rank_of(sq) == 6)  
        moves |= bb_down(one) & empty;
    }
    moves |= pawn_attacks(sq, BLACK) & ~friendly;
  }

  return moves;
}

inline Bitboard knight_attacks(Square sq) {
  Bitboard bb = square_bb(sq);
  Bitboard l1 = (bb >> 1) & ~FILE_H;
  Bitboard l2 = (bb >> 2) & ~(FILE_H | (FILE_H >> 1));
  Bitboard r1 = (bb << 1) & ~FILE_A;
  Bitboard r2 = (bb << 2) & ~(FILE_A | (FILE_A << 1));

  return (l2 << 8) | (l2 >> 8) | (r2 << 8) | (r2 >> 8) |
         (l1 << 16) | (l1 >> 16) | (r1 << 16) | (r1 >> 16);
}

inline Bitboard knight_moves(Square sq, Bitboard friendly) {
  return knight_attacks(sq) & ~friendly;
}

inline Bitboard king_attacks(Square sq) {
  Bitboard bb = square_bb(sq);
  return bb_up(bb) | bb_down(bb) | bb_left(bb) | bb_right(bb) |
         bb_up_left(bb) | bb_up_right(bb) |
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

}
