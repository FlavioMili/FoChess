// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#include "board.h"


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

void Board::makeMove(const Move& m) {
  Square from = m.from_sq(), to = m.to_sq();
  Color us = sideToMove, them = Color(BLACK - us);
  Piece pt = piece_on(from);
  auto mt = m.type();

  const Bitboard from_bb = Bitboards::square_bb(from);
  const Bitboard to_bb = Bitboards::square_bb(to);
  const Bitboard from_to_bb = (from_bb | to_bb);

  // Clear en passant (will be set again if this is a double pawn push)
  enPassant = NO_SQUARE;
  captured_piece = NO_PIECE;

  const Bitboard is_capture = (occupancy[them] & to_bb);


  // Update half-move clock (reset on pawn move or capture)
  halfMoveClock = (pt == PAWN || is_capture) ? 0 : halfMoveClock + 1;

  if (is_capture) {
    captured_piece = piece_on(to);
    pieces[them][captured_piece] &= ~to_bb;

    // Update castling rights if rook captured
    if (captured_piece == ROOK) [[unlikely]] {
      if (to == Square::A1) castling.whiteQueenside = false;
      else if (to == Square::H1) castling.whiteKingside = false;
      else if (to == Square::A8) castling.blackQueenside = false;
      else if (to == Square::H8) castling.blackKingside = false;
    }
  }

  // Move the piece **This uses some xor trick**
  pieces[us][pt] ^= from_to_bb;

  // Handle special moves
  if (mt != MoveType::NORMAL) [[unlikely]] {
    switch (mt) {
      case MoveType::PROMOTION:
        // Remove pawn, add promoted piece
        pieces[us][PAWN] &= ~to_bb;
        pieces[us][m.promotion_type()] |= to_bb;
        break;

      case MoveType::EN_PASSANT: {
        Square capturedSq = (us == WHITE) ? Bitboards::down(to) : Bitboards::up(to);
        captured_piece = PAWN;
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
  } // end not normal movetype checks

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
    if (us == WHITE) {
      // White double-step from rank 2
      if ((from_bb & Bitboards::RANK_2) && to == Bitboards::up(Bitboards::up(from))) {
        enPassant = Bitboards::up(from);  // the square behind the pawn
      }
    } else { // BLACK
      // Black double-step from rank 7
      if ((from_bb & Bitboards::RANK_7) && to == Bitboards::down(Bitboards::down(from))) {
        enPassant = Bitboards::down(from);  // the square behind the pawn
      }
    }
  }

  updateOccupancy();

  fullMoveNumber += BLACK;
  sideToMove = them;
}
