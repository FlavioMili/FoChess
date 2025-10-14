// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#include "zobrist.h"

#include <cstddef>

#include "bitboard.h"
#include "board.h"
#include "helpers.h"
#include "types.h"

namespace Zobrist {

Bitboard generate_hash(const Board& board) {
  Bitboard hash = 0;

  for (size_t c = WHITE; c <= BLACK; ++c) {
    for (size_t p = PAWN; p <= KING; ++p) {
      Bitboard bb = board.pieces[c][p];
      while (bb) {
        size_t sq = static_cast<size_t>(__builtin_ctzll(bb));
        bb &= bb - 1;
        hash ^= pieces_keys[piece_to_idx(c, p, sq)];
      }
    }
  }

  if (board.enPassant != NO_SQUARE) hash ^= enPassant_keys[board.enPassant];

  hash ^= castling_keys[PrintingHelpers::encode_castling(board.castling)];

  if (board.sideToMove == BLACK) hash ^= sideToMove_key;

  return hash;
}

}  // namespace Zobrist
