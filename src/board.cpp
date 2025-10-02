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

static char pieceChar(Piece pt, Color c) {
  char ch = '?';
  switch (pt) {
    case PAWN:
      ch = 'P';
      break;
    case KNIGHT:
      ch = 'N';
      break;
    case BISHOP:
      ch = 'B';
      break;
    case ROOK:
      ch = 'R';
      break;
    case QUEEN:
      ch = 'Q';
      break;
    case KING:
      ch = 'K';
      break;
    default:
      ch = '-';
      break;
  }
  if (c == BLACK) ch = static_cast<char>(std::tolower(ch));
  return ch;
}

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
            symbol = pieceChar(static_cast<Piece>(p), static_cast<Color>(c));
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
