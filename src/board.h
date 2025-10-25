// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#pragma once
#include <array>
#include <bit>
#include <cstdint>

#include "bitboard.h"
#include "magic.h"
#include "move.h"
#include "types.h"

// I made some tests with an enum : uint8_t
// but this implementation was much faster
// apparently so I will keep it like this
struct CastlingRights {
  bool whiteKingside = false;
  bool whiteQueenside = false;
  bool blackKingside = false;
  bool blackQueenside = false;

  void clear() {
    whiteKingside = whiteQueenside = blackKingside = blackQueenside = false;
  }
};

struct Board {
  Board();
  Board(const Board& other) = default;

  void updateOccupancy();

  Bitboard attacks_to(Square sq, Color attacker_color) const;

  inline bool is_in_check(Color c) const noexcept
      __attribute__((always_inline));

  Color color_on(Square sq) const;
  Piece piece_on(Square sq) const;

  void makeMove(const Move& m);
  inline bool isLegalMove(const Move& m) const;
  inline bool moveExists(const Move& m) const;

  std::array<std::array<Bitboard, 6>, 2> pieces;  // [color][pieceType]
  std::array<Bitboard, 2> occupancy;              // white/black
  uint64_t hash;
  Bitboard allPieces;                             // all occupied squares
  CastlingRights castling;                        // castling rights flags
  uint16_t fullMoveNumber;
  std::array<Square, 2> kingSq;
  uint8_t halfMoveClock;
  Square enPassant = NO_SQUARE;  // en passant target
  Color sideToMove;
  Piece captured_piece;
};

inline void Board::updateOccupancy() {
  occupancy[WHITE] = pieces[WHITE][PAWN] | pieces[WHITE][KNIGHT] |
                     pieces[WHITE][BISHOP] | pieces[WHITE][ROOK] |
                     pieces[WHITE][QUEEN] | pieces[WHITE][KING];

  occupancy[BLACK] = pieces[BLACK][PAWN] | pieces[BLACK][KNIGHT] |
                     pieces[BLACK][BISHOP] | pieces[BLACK][ROOK] |
                     pieces[BLACK][QUEEN] | pieces[BLACK][KING];

  allPieces = occupancy[WHITE] | occupancy[BLACK];
  kingSq = {static_cast<Square>(std::countr_zero(pieces[WHITE][KING])),
            static_cast<Square>(std::countr_zero(pieces[BLACK][KING]))};
}

inline Bitboard Board::attacks_to(Square sq, Color attacker_color) const {
  Bitboard attackers = 0;

  attackers |= Bitboards::pawn_attacks_mask(
                   sq, static_cast<Color>(BLACK - attacker_color)) &
               pieces[attacker_color][PAWN];
  attackers |= Bitboards::knight_attacks(sq) & pieces[attacker_color][KNIGHT];
  attackers |= Bitboards::king_attacks(sq) & pieces[attacker_color][KING];
  attackers |= Bitboards::bishop_attacks(sq, allPieces) &
               (pieces[attacker_color][BISHOP] | pieces[attacker_color][QUEEN]);
  attackers |= Bitboards::rook_attacks(sq, allPieces) &
               (pieces[attacker_color][ROOK] | pieces[attacker_color][QUEEN]);

  return attackers;
}

inline __attribute__((always_inline)) bool Board::is_in_check(
    Color c) const noexcept {
  // Bitboard king_bb = pieces[c][KING];
  //
  // Square kingSquare = static_cast<Square>(__builtin_ctzll(king_bb));
  Color enemy = Color(BLACK - c);
  return attacks_to(kingSq[c], enemy) != 0;
}

inline Color Board::color_on(Square sq) const {
  Bitboard sq_bb = Bitboards::square_bb(sq);
  if (occupancy[WHITE] & sq_bb) return WHITE;
  if (occupancy[BLACK] & sq_bb) return BLACK;
  return NO_COLOR;
}

inline Piece Board::piece_on(Square sq) const {
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

inline bool Board::moveExists(const Move& m) const {
  auto to = m.to_sq();
  return ((m.from_sq() & occupancy[sideToMove]) &&
          ((to & ~allPieces) || (to & occupancy[BLACK - sideToMove])));
}

inline bool Board::isLegalMove(const Move& m) const {
  const Square from = m.from_sq(), to = m.to_sq();
  const Color us = sideToMove, them = Color(BLACK - us);
  const Piece pt = piece_on(from);
  const auto mt = m.type();

  const Bitboard from_bb = Bitboards::square_bb(from);
  const Bitboard to_bb = Bitboards::square_bb(to);
  const Bitboard from_to_bb = (from_bb | to_bb);

  // Copies we will mutate to simulate the move
  auto pcs = pieces;             // [color][piece]
  auto occ = occupancy;          // [color]
  Bitboard occ_all = allPieces;  // union

  // handle capture (normal capture)
  const Bitboard is_capture = (occ[them] & to_bb);
  if (is_capture) {
    Piece capturedP = piece_on(to);
    pcs[them][capturedP] &= ~to_bb;
    occ[them] ^= to_bb;
    occ_all ^= to_bb;
  }

  // move the piece
  pcs[us][pt] ^= from_to_bb;
  occ[us] ^= from_to_bb;
  occ_all ^= from_to_bb;

  // handle special moves
  if (mt != MoveType::NORMAL) [[unlikely]] {
    switch (mt) {
      case MoveType::EN_PASSANT: {
        Square capturedSq =
            (us == WHITE) ? Bitboards::down(to) : Bitboards::up(to);
        Bitboard capturedSq_bb = Bitboards::square_bb(capturedSq);
        pcs[them][PAWN] &= ~capturedSq_bb;
        occ[them] ^= capturedSq_bb;
        occ_all ^= capturedSq_bb;
        break;
      }
      case MoveType::CASTLING: {
        Square rookFrom, rookTo;
        if (to > from) {  // kingside
          rookFrom = (us == WHITE) ? Square::H1 : Square::H8;
          rookTo = (us == WHITE) ? Square::F1 : Square::F8;
        } else {  // queenside
          rookFrom = (us == WHITE) ? Square::A1 : Square::A8;
          rookTo = (us == WHITE) ? Square::D1 : Square::D8;
        }
        Bitboard r_bb =
            (Bitboards::square_bb(rookFrom) | Bitboards::square_bb(rookTo));
        pcs[us][ROOK] ^= r_bb;
        occ[us] ^= r_bb;
        occ_all ^= r_bb;
        break;
      }
      default:
        break;
    }
  }

  Square king_sq = kingSq[us];
  if (pt == KING) king_sq = to;

  // Bishop/Queen diagonal
  if (Bitboards::bishop_attacks(king_sq, occ_all) &
      (pcs[them][BISHOP] | pcs[them][QUEEN]))
    return false;

  // Rook/Queen straight
  if (Bitboards::rook_attacks(king_sq, occ_all) &
      (pcs[them][ROOK] | pcs[them][QUEEN]))
    return false;

  // Knight attackers
  if (Bitboards::knight_attacks(king_sq) & pcs[them][KNIGHT]) return false;

  // Pawn attackers
  if (Bitboards::pawn_attacks_mask(king_sq, us) & pcs[them][PAWN]) return false;

  if (Bitboards::king_attacks(king_sq) & pcs[them][KING]) return false;

  return true;
}
