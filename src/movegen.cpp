// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#include "movegen.h"

#include <cassert>
#include <cstddef>
#include "bitboard.h"
#include "types.h"
using namespace Bitboards;

 // TODO check for movetype
void MoveGen::add_move(Square from, Square to, Piece promo) {
  assert(move_count < MAX_MOVES);
  move_list[move_count++] = Move().make<MoveType::NORMAL>(from, to, promo);
}

void MoveGen::generate_all() {
  move_count = 0;
  Bitboard occ = board.occupancy[WHITE] | board.occupancy[BLACK];

  // TODO change this to take the current static state so that it calculates only one of the two colours
  for (size_t color = WHITE; color <= BLACK; ++color) {

    Bitboard friendly = board.occupancy[color];
    for (size_t piece = PAWN; piece <= KING; ++piece) {
      Bitboard bb = board.pieces[color][piece];
      while (bb) {
        Square from = static_cast<Square>(__builtin_ctzll(bb));
        Bitboard targets = 0;

        switch (piece) {
          case PAWN:
            targets = pawn_moves(from, static_cast<Color>(color), ~occ, friendly);
            break;
          case KNIGHT:
            targets = knight_moves(from, friendly);
            break;
          case BISHOP:
            targets = bishop_moves(from, occ, friendly);
            break;
          case ROOK:
            targets = rook_moves(from, occ, friendly);
            break;
          case QUEEN:
            targets = queen_moves(from, occ, friendly);
            break;
          case KING:
            targets = king_moves(from, friendly);
            break;
        }

        // Add moves
        while (targets) {
          Square to = static_cast<Square>(__builtin_ctzll(targets));
          add_move(from, to, static_cast<Piece>(piece));
          targets &= targets - 1;
        }

        bb &= bb - 1;  // clear LSB
      }
    }
  }
}
