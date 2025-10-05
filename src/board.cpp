// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#include "board.h"

#include <cstddef>
#include <iostream>

#include "types.h"


// ---------------- Constructor ----------------
Board::Board()
    : occupancy{0, 0},
      allPieces(0),
      sideToMove(Color::WHITE),
      castling(),
      enPassant(Square::NO_SQUARE),
      halfMoveClock(0),
      fullMoveNumber(1) {
  pieces.fill({});
}

// ---------------- Accessors ----------------
Bitboard Board::getPieceBitboard(Color c, Piece pt) const {
  return pieces[c][pt];
}

Bitboard Board::getOccupancy(Color c) const {
  return occupancy[c];
}
Bitboard Board::getAllOccupied() const {
  return allPieces;
}
Color Board::getSideToMove() const {
  return sideToMove;
}
Square Board::getEnPassant() const {
  return enPassant;
}
uint16_t Board::getHalfMoveClock() const {
  return halfMoveClock;
}
uint16_t Board::getFullMoveNumber() const {
  return fullMoveNumber;
}
CastlingRights Board::getCastlingRights() const {
  return castling;
}

void Board::setPiece(Color c, Piece pt, Bitboard bb) {
  pieces[c][pt] = bb;
  updateOccupancy();
}

void Board::setSideToMove(Color c) {
  sideToMove = c;
}
void Board::setEnPassant(Square sq) {
  enPassant = sq;
}
void Board::setCastlingRights(CastlingRights rights) {
  castling = rights;
}

void Board::incrementHalfMove() {
  ++halfMoveClock;
}
void Board::incrementFullMove() {
  ++fullMoveNumber;
}

void Board::clear() {
  pieces.fill({});
  occupancy[Color::WHITE] = occupancy[Color::BLACK] = allPieces = 0;
  sideToMove = Color::WHITE;
  castling.clear();
  enPassant = Square::NO_SQUARE;
  halfMoveClock = 0;
  fullMoveNumber = 1;
}

void Board::updateOccupancy() {
  occupancy[Color::WHITE] = occupancy[Color::BLACK] = 0;
  allPieces = 0;
  for (size_t c = 0; c < 2; ++c) {
    for (size_t pt = 0; pt < 6; ++pt) {
      occupancy[c] |= pieces[c][pt];
      allPieces |= pieces[c][pt];
    }
  }
}

void Board::printBoard() const {
  for (int rank = 7; rank >= 0; --rank) {
    std::cout << rank + 1 << "  ";
    for (int file = 0; file < 8; ++file) {
      int sq = rank * 8 + file;  // A8=0, H1=63
      Bitboard mask = 1ULL << sq;

      char symbol = '.';
      for (size_t c = 0; c < 2; ++c) {
        for (size_t p = 0; p < 6; ++p) {
          if (pieces[c][p] & mask) {
            symbol = PrintingHelpers::pieceChar(static_cast<Piece>(p), static_cast<Color>(c));
            goto print_done;
          }
        }
      }
    print_done:
      std::cout << symbol << ' ';
    }
    std::cout << '\n';
  }
  std::cout << "\n   a b c d e f g h\n";
}


inline Bitboard Board::attacks_to(Square sq) const {
  Bitboard attackers = 0;
  Bitboard occ = occupancy[WHITE] | occupancy[BLACK];

  for (size_t color = WHITE; color <= BLACK; ++color) {
    for (size_t piece = PAWN; piece <= KING; ++piece) {
      Bitboard bb = pieces[color][piece];
      while (bb) {
        Square from = static_cast<Square>(__builtin_ctzll(bb));  // index of LSB
        Bitboard attacks = 0;

        switch (piece) {
          case PAWN:   attacks = Bitboards::pawn_attacks(from, static_cast<Color>(color)); break;
          case KNIGHT: attacks = Bitboards::knight_attacks(from); break;
          case BISHOP: attacks = Bitboards::bishop_attacks(from, occ); break;
          case ROOK:   attacks = Bitboards::rook_attacks(from, occ); break;
          case QUEEN:  attacks = Bitboards::queen_attacks(from, occ); break;
          case KING:   attacks = Bitboards::king_attacks(from); break;
        }

        if (attacks & Bitboards::square_bb(sq)) {
          attackers |= Bitboards::square_bb(from);
        }

        bb &= bb - 1; // clear LSB
      }
    }
  }

  return attackers;
}


Color Board::color_on(Square sq) const {
  Bitboard sq_bb = Bitboards::square_bb(sq);
  if (occupancy[WHITE] & sq_bb) return WHITE;
  if (occupancy[BLACK] & sq_bb) return BLACK;
  return NO_COLOR;
}

Piece Board::piece_on(Square sq) const {
  Bitboard sq_bb = Bitboards::square_bb(sq);

  for (size_t color = WHITE; color <= BLACK; ++color) {
    for (size_t pt = PAWN; pt <= KING; ++pt) {
      if (pieces[color][pt] & sq_bb) {
        return static_cast<Piece>(pt);
      }
    }
  }
  return NO_PIECE;
}
