// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#include "board.h"

#include <cstddef>

#include "bitboard.h"
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

Bitboard Board::attacks_to(Square sq, Color attacker_color) const {
  const Bitboard occ = occupancy[WHITE] | occupancy[BLACK];
  const Bitboard target_bb = Bitboards::square_bb(sq);
  Bitboard attackers = 0;

  attackers |= Bitboards::pawn_attacks_mask(sq, static_cast<Color>(BLACK - attacker_color)) &
               pieces[attacker_color][PAWN];
  attackers |= Bitboards::knight_attacks(sq) & pieces[attacker_color][KNIGHT];
  attackers |= Bitboards::king_attacks(sq) & pieces[attacker_color][KING];
  attackers |= Bitboards::bishop_attacks(sq, occ) &
               (pieces[attacker_color][BISHOP] | pieces[attacker_color][QUEEN]);
  attackers |= Bitboards::rook_attacks(sq, occ) &
               (pieces[attacker_color][ROOK] | pieces[attacker_color][QUEEN]);

  return attackers;
}

bool Board::is_in_check(Color c) const {
  Bitboard king_bb = pieces[c][KING];
  if (!king_bb) return false;

  Square kingSquare = static_cast<Square>(__builtin_ctzll(king_bb));
  Color enemy = Color(BLACK - c);

  return attacks_to(kingSquare, enemy) != 0;
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

void Board::makeMove(const Move& m) {
  Square from = m.from_sq();
  Square to = m.to_sq();
  Color us = sideToMove;
  Color them = Color(BLACK - us);
  Piece pt = piece_on(from);

  // Update half-move clock (reset on pawn move or capture)
  if (pt == PAWN || (occupancy[them] & Bitboards::square_bb(to))) {
    halfMoveClock = 0;
  } else {
    halfMoveClock++;
  }

  // Clear en passant (will be set again if this is a double pawn push)
  enPassant = NO_SQUARE;

  // Handle captures (remove captured piece)
#ifdef DEBUG
  was_captured = false;
#endif
  if (occupancy[them] & Bitboards::square_bb(to)) {
    Piece captured = piece_on(to);
    pieces[them][captured] &= ~Bitboards::square_bb(to);

    // Update castling rights if rook captured
    // Bit 0 = A8, Bit 7 = H8, Bit 56 = A1, Bit 63 = H1
    if (captured == ROOK) {
      if (to == 56)
        castling.whiteQueenside = false;
      else if (to == 63)
        castling.whiteKingside = false;
      else if (to == 0)
        castling.blackQueenside = false;
      else if (to == 7)
        castling.blackKingside = false;
    }
#ifdef DEBUG
    was_captured = true;
#endif
  }

  // Move the piece
  pieces[us][pt] &= ~Bitboards::square_bb(from);  // Remove from source
  pieces[us][pt] |= Bitboards::square_bb(to);     // Place at destination

  // Handle special moves
  switch (m.type()) {
    case MoveType::NORMAL:
      break;

    case MoveType::PROMOTION:
      // Remove pawn, add promoted piece
      pieces[us][PAWN] &= ~Bitboards::square_bb(to);
      pieces[us][m.promotion_type()] |= Bitboards::square_bb(to);
      break;

    case MoveType::EN_PASSANT: {
      // Remove the captured pawn (which is not on the 'to' square)
      // White pawns move down (increasing square numbers)
      // Black pawns move up (decreasing square numbers)
      Square capturedSq = (us == WHITE) ? Bitboards::down(to) : Bitboards::up(to);
      pieces[them][PAWN] &= ~Bitboards::square_bb(capturedSq);
#ifdef DEBUG
      was_captured = true;
#endif
      break;
    }

    case MoveType::CASTLING: {
      // Move the rook
      Square rookFrom, rookTo;
      if (to > from) {                                      // Kingside (towards H-file)
        rookFrom = (us == WHITE) ? Square(63) : Square(7);  // H1 : H8
        rookTo = (us == WHITE) ? Square(61) : Square(5);    // F1 : F8
      } else {                                              // Queenside (towards A-file)
        rookFrom = (us == WHITE) ? Square(56) : Square(0);  // A1 : A8
        rookTo = (us == WHITE) ? Square(59) : Square(3);    // D1 : D8
      }
      pieces[us][ROOK] &= ~Bitboards::square_bb(rookFrom);
      pieces[us][ROOK] |= Bitboards::square_bb(rookTo);
      break;
    }
  }

  // Update castling rights based on piece moved
  if (pt == KING) {
    if (us == WHITE) {
      castling.whiteKingside = castling.whiteQueenside = false;
    } else {
      castling.blackKingside = castling.blackQueenside = false;
    }
  } else if (pt == ROOK) {
    if (from == Square::A1)
      castling.whiteQueenside = false;
    else if (from == Square::H1)
      castling.whiteKingside = false;
    else if (from == Square::A8)
      castling.blackQueenside = false;
    else if (from == Square::H8)
      castling.blackKingside = false;
  }

  if (pt == PAWN) {
    if (us == WHITE && to == Bitboards::up(Bitboards::up(from))) {
      enPassant = Bitboards::up(from);
    } else if (us == BLACK && to == Bitboards::down(Bitboards::down(from))) {
      enPassant = Bitboards::down(from);
    }
  }

  updateOccupancy();

  if (us == BLACK) {
    fullMoveNumber++;
  }
  sideToMove = them;
}
