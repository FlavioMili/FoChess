// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2026 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#include "board.h"

#include "bitboard.h"
#include "helpers.h"
#include "move.h"
#include "types.h"
#include "zobrist.h"

void Board::makeNullMove() {
  if (enPassant != NO_SQUARE) {
    hash ^= Zobrist::enPassant_keys[enPassant];
    enPassant = NO_SQUARE;
  }
  hash ^= Zobrist::sideToMove_key;
#ifdef DEBUG
  captured_piece = NO_PIECE;
#endif
  sideToMove = static_cast<Color>(BLACK - sideToMove);
}

void Board::makeMove(const Move& m) {
  const Square from = m.from_sq();
  const Square to = m.to_sq();
  const Color us = sideToMove;
  const auto them = Color(BLACK - us);
  const Piece pt = piece_on(from);
  const auto mt = m.type();

  const Square old_ep = enPassant;

  uint64_t new_hash = hash ^ Zobrist::sideToMove_key;
  if (old_ep != NO_SQUARE) new_hash ^= Zobrist::enPassant_keys[old_ep];

  const Bitboard from_bb = Bitboards::square_bb(from);
  const Bitboard to_bb = Bitboards::square_bb(to);
  const Bitboard from_to_bb = (from_bb | to_bb);

  enPassant = NO_SQUARE;
  Piece captured = NO_PIECE;
  const Bitboard is_capture = (occupancy[them] & to_bb);

  halfMoveClock = (pt == PAWN || is_capture) ? 0 : halfMoveClock + 1;

  if (is_capture) [[unlikely]] {
    captured = piece_on(to);
    pieces[them][captured] ^= to_bb;
    occupancy[them] ^= to_bb;
    allPieces ^= to_bb;
    new_hash ^=
        Zobrist::pieces_keys[Zobrist::piece_to_idx(them, captured, to)];
  }

  pieces[us][pt] ^= from_to_bb;
  squares[from] = NO_PIECE;
  squares[to] = pt;
  occupancy[us] ^= from_to_bb;
  allPieces ^= from_to_bb;
  new_hash ^= Zobrist::pieces_keys[Zobrist::piece_to_idx(us, pt, from)];
  new_hash ^= Zobrist::pieces_keys[Zobrist::piece_to_idx(us, pt, to)];

  if (mt != MoveType::NORMAL) {
    switch (mt) {
      case MoveType::PROMOTION:
        pieces[us][PAWN] ^= to_bb;
        pieces[us][m.promotion_type()] |= to_bb;
        squares[to] = m.promotion_type();
        new_hash ^= Zobrist::pieces_keys[Zobrist::piece_to_idx(us, pt, to)];
        new_hash ^= Zobrist::pieces_keys[Zobrist::piece_to_idx(
            us, m.promotion_type(), to)];
        break;

      case MoveType::EN_PASSANT: {
        Square capturedSq =
            (us == WHITE) ? Bitboards::down(to) : Bitboards::up(to);
        Bitboard capturedSq_bb = Bitboards::square_bb(capturedSq);
        captured = PAWN;
        pieces[them][PAWN] ^= capturedSq_bb;
        squares[capturedSq] = NO_PIECE;
        occupancy[them] ^= capturedSq_bb;
        allPieces ^= capturedSq_bb;
        new_hash ^=
            Zobrist::pieces_keys[Zobrist::piece_to_idx(them, PAWN, capturedSq)];
        break;
      }

      case MoveType::CASTLING: {
        Square rookFrom;
        Square rookTo;
        if (to > from) {  // Kingside
          rookFrom = (us == WHITE) ? H1 : H8;
          rookTo = (us == WHITE) ? F1 : F8;
        } else {  // Queenside
          rookFrom = (us == WHITE) ? A1 : A8;
          rookTo = (us == WHITE) ? D1 : D8;
        }
        Bitboard rook_from_to =
            (Bitboards::square_bb(rookFrom) | Bitboards::square_bb(rookTo));
        pieces[us][ROOK] ^= rook_from_to;
        squares[rookFrom] = NO_PIECE;
        squares[rookTo] = ROOK;
        occupancy[us] ^= rook_from_to;
        allPieces ^= rook_from_to;
        new_hash ^=
            Zobrist::pieces_keys[Zobrist::piece_to_idx(us, ROOK, rookFrom)];
        new_hash ^=
            Zobrist::pieces_keys[Zobrist::piece_to_idx(us, ROOK, rookTo)];
        break;
      }
      default:
        break;
    }
  }

  const bool may_change_castling =
      pt == KING || pt == ROOK || captured == ROOK;
  const uint64_t old_castling_key =
      may_change_castling
          ? Zobrist::castling_keys[PrintingHelpers::encode_castling(castling)]
          : 0;

  if (pt == KING) {
    if (us == WHITE) {
      castling.whiteKingside = castling.whiteQueenside = false;
      kingSq[WHITE] = to;
    } else {
      castling.blackKingside = castling.blackQueenside = false;
      kingSq[BLACK] = to;
    }
  } else if (pt == ROOK) {
    if (from == A1) castling.whiteQueenside = false;
    else if (from == H1) castling.whiteKingside = false;
    else if (from == A8) castling.blackQueenside = false;
    else if (from == H8) castling.blackKingside = false;
  }

  if (captured == ROOK) {
    if (to == A1) castling.whiteQueenside = false;
    else if (to == H1) castling.whiteKingside = false;
    else if (to == A8) castling.blackQueenside = false;
    else if (to == H8) castling.blackKingside = false;
  }

  if (pt == PAWN) {
    if (us == WHITE) {
      if ((from_bb & Bitboards::RANK_2) &&
          to == Bitboards::up(Bitboards::up(from))) {
        enPassant = Bitboards::up(from);
      }
    } else {
      if ((from_bb & Bitboards::RANK_7) &&
          to == Bitboards::down(Bitboards::down(from))) {
        enPassant = Bitboards::down(from);
      }
    }
  }

  if (enPassant != NO_SQUARE)
    new_hash ^= Zobrist::enPassant_keys[enPassant];
  if (may_change_castling) {
    const uint64_t new_castling_key =
        Zobrist::castling_keys[PrintingHelpers::encode_castling(castling)];
    new_hash ^= old_castling_key ^ new_castling_key;
  }

  fullMoveNumber += us;
  sideToMove = them;
#ifdef DEBUG
  captured_piece = captured;
#endif
  hash = new_hash;
}
