// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#include "board.h"

#include <cstddef>

#include "bitboard.h"
#include "move.h"
#include "types.h"

// ---------------- Constructor ----------------
Board::Board()
  : pieces({}),
  occupancy{0, 0},
  allPieces(0),
  castling(),
  fullMoveNumber(0),
  halfMoveClock(0),
  enPassant(Square::NO_SQUARE),
  sideToMove(Color::WHITE) {}

void Board::updateOccupancy() {
  occupancy[WHITE] = pieces[WHITE][PAWN] | pieces[WHITE][KNIGHT] | pieces[WHITE][BISHOP] |
                     pieces[WHITE][ROOK] | pieces[WHITE][QUEEN] | pieces[WHITE][KING];

  occupancy[BLACK] = pieces[BLACK][PAWN] | pieces[BLACK][KNIGHT] | pieces[BLACK][BISHOP] |
                     pieces[BLACK][ROOK] | pieces[BLACK][QUEEN] | pieces[BLACK][KING];

  allPieces = occupancy[WHITE] | occupancy[BLACK];
}

Bitboard Board::attacks_to(Square sq, Color attacker_color) const {
  Bitboard attackers = 0;

  attackers |= Bitboards::pawn_attacks_mask(sq, static_cast<Color>(BLACK - attacker_color)) &
    pieces[attacker_color][PAWN];
  attackers |= Bitboards::knight_attacks(sq) & pieces[attacker_color][KNIGHT];
  attackers |= Bitboards::king_attacks(sq) & pieces[attacker_color][KING];
  attackers |= Bitboards::bishop_attacks(sq, allPieces) &
    (pieces[attacker_color][BISHOP] | pieces[attacker_color][QUEEN]);
  attackers |= Bitboards::rook_attacks(sq, allPieces) &
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
  Square from = m.from_sq(), to = m.to_sq();
  Color us = sideToMove, them = Color(BLACK - us);
  Piece pt = piece_on(from);

  const Bitboard from_bb = Bitboards::square_bb(from);
  const Bitboard to_bb = Bitboards::square_bb(to);
  const Bitboard from_to_bb = (from_bb | to_bb);

  const bool is_capture = (occupancy[them] & to_bb) != 0;

#ifdef DEBUG
  was_captured = is_capture;
#endif

  // Update half-move clock (reset on pawn move or capture)
  halfMoveClock = (pt == PAWN || is_capture) ? 0 : halfMoveClock + 1;

  // Clear en passant (will be set again if this is a double pawn push)
  enPassant = NO_SQUARE;

  if (is_capture) {
    Piece captured = piece_on(to);
    pieces[them][captured] &= ~to_bb;

    // Update castling rights if rook captured
    if (captured == ROOK) {
      if (to == Square::A1) castling.whiteQueenside = false;
      else if (to == Square::H1) castling.whiteKingside = false;
      else if (to == Square::A8) castling.blackQueenside = false;
      else if (to == Square::H8) castling.blackKingside = false;
    }
  }

  // Move the piece **This uses some xor trick**
  pieces[us][pt] ^= from_to_bb;

  // Handle special moves
  auto mt = m.type();
  if (mt != MoveType::NORMAL) {

    switch (mt) {
      case MoveType::PROMOTION:
        // Remove pawn, add promoted piece
        pieces[us][PAWN] &= ~to_bb;
        pieces[us][m.promotion_type()] |= to_bb;
        break;

      case MoveType::EN_PASSANT: {
        Square capturedSq = (us == WHITE) ? Bitboards::down(to) : Bitboards::up(to);
        pieces[them][PAWN] &= ~Bitboards::square_bb(capturedSq);
        break;
      }

      case MoveType::CASTLING: {
        // Move the rook
        Square rookFrom, rookTo;
        if (to > from) {  // Kingside
          rookFrom = (us == WHITE) ? Square::H1 : Square::H8;
          rookTo = (us == WHITE) ? Square::F1 : Square::F8;
        } else {  // Queenside
          rookFrom = (us == WHITE) ? Square::A1 : Square::A8;
          rookTo = (us == WHITE) ? Square::D1 : Square::D8;
        }

        // small xor trick
        pieces[us][ROOK] ^= (Bitboards::square_bb(rookFrom) | 
                              Bitboards::square_bb(rookTo));
        break;
      }

      default: break;
    }
  }

  // Update castling rights based on piece moved
  if (pt == KING) {
    if (us == WHITE) {
      castling.whiteKingside = castling.whiteQueenside = false;
    } else {
      castling.blackKingside = castling.blackQueenside = false;
    }
  } 
  else if (pt == ROOK) {
    if (from == Square::A1) castling.whiteQueenside = false;
    else if (from == Square::H1) castling.whiteKingside = false;
    else if (from == Square::A8) castling.blackQueenside = false;
    else if (from == Square::H8) castling.blackKingside = false;

  } 
  else if (pt == PAWN) {  // check for en passant
    if (us == WHITE && to == Bitboards::up(Bitboards::up(from))) {
      enPassant = Bitboards::up(from);
    } else if (us == BLACK && to == Bitboards::down(Bitboards::down(from))) {
      enPassant = Bitboards::down(from);
    }
  }

  updateOccupancy();

  fullMoveNumber += BLACK;

  sideToMove = them;
}
