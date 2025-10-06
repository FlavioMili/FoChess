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
  Bitboard getPieceBitboard(Color c, Piece pt) const;
  Bitboard getOccupancy(Color c) const;
  Bitboard getAllOccupied() const;
  Color getSideToMove() const;
  Square getEnPassant() const;
  uint16_t getHalfMoveClock() const;
  uint16_t getFullMoveNumber() const;
  CastlingRights getCastlingRights() const;

  void setPiece(Color c, Piece pt, Bitboard bb);
  void setSideToMove(Color c);
  void setEnPassant(Square sq);
  void setCastlingRights(CastlingRights rights);
  void incrementHalfMove();
  void incrementFullMove();
  void clear();

  Color color_on(Square sq) const;
  Piece piece_on(Square sq) const;

  Bitboard attacks_to(Square sq) const;

  void makeMove(const Move& m);

  std::array<std::array<Bitboard, 6>, 2> pieces;  // [color][pieceType]
  std::array<Bitboard, 2> occupancy;              // white/black
  Bitboard allPieces;                             // all occupied squares
  Color sideToMove;
  CastlingRights castling;  // castling rights flags
  Square enPassant;  // en passant target
  uint16_t halfMoveClock;
  uint16_t fullMoveNumber;

  void updateOccupancy();
};

