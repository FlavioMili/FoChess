// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#pragma once
#include <array>
#include <cstdint>

#include "bitboard.h"
#include "move.h"
#include "types.h"

struct CastlingRights {
  bool whiteKingside = false;
  bool whiteQueenside = false;
  bool blackKingside = false;
  bool blackQueenside = false;

  void clear() {
    whiteKingside = whiteQueenside = 
      blackKingside = blackQueenside = false;
  }
};

struct Board {
  Board();
  Board(const Board& other) = default;

  void updateOccupancy();

  Bitboard attacks_to(Square sq, Color attacker_color) const;

  bool is_in_check(Color c) const;

  Color color_on(Square sq) const;
  Piece piece_on(Square sq) const;

  void makeMove(const Move& m);

  std::array<std::array<Bitboard, 6>, 2> pieces;  // [color][pieceType]
  std::array<Bitboard, 2> occupancy;              // white/black
  // std::array<Piece, 64> square_to_piece; TODO implement
  Bitboard allPieces;                             // all occupied squares
  CastlingRights castling;  // castling rights flags
  uint16_t fullMoveNumber;
  uint8_t halfMoveClock;
  Square enPassant = NO_SQUARE;  // en passant target
  Color sideToMove;

#ifdef DEBUG
  bool was_captured;
#endif
};
