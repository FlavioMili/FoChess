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
  Board() = default;
  Board(const Board& other) = default;

  void updateOccupancy();

  Bitboard attacks_to(Square sq, Color attacker_color) const;

  bool is_in_check(Color c) const noexcept;

  Color color_on(Square sq) const;
  Piece piece_on(Square sq) const;

  void makeMove(const Move& m);
  // Search-only pass: clears en passant and flips the side/hash, but does not
  // advance game clocks because it is not a legal move in the game history.
  void makeNullMove();
  // Requires a pseudo-legal move whose source belongs to sideToMove.
  bool isLegalMove(const Move& m) const;
  bool moveExists(const Move& m) const;

  std::array<std::array<Bitboard, 6>, 2> pieces {};  // [color][pieceType]
  std::array<Bitboard, 2> occupancy {0,0};              // white/black
  uint64_t hash;
  Bitboard allPieces {};       // all occupied squares
  CastlingRights castling {};  // castling rights flags
  uint16_t fullMoveNumber {1};
  std::array<Square, 2> kingSq {NO_SQUARE, NO_SQUARE};
  uint16_t halfMoveClock {};
  Square enPassant = NO_SQUARE;  // en passant target
  Color sideToMove {WHITE};
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
  auto enemy = static_cast<Color>(BLACK - c);
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
  if (m.raw() == EMPTY_MOVE) return false;

  const Bitboard from_bb = Bitboards::square_bb(m.from_sq());
  const Bitboard to_bb = Bitboards::square_bb(m.to_sq());
  return (from_bb & occupancy[sideToMove]) != 0 &&
         (to_bb & occupancy[sideToMove]) == 0;
}

inline bool Board::isLegalMove(const Move& m) const {
  const Square from = m.from_sq();
  const Square to = m.to_sq();
  const Color us = sideToMove;
  const auto them = Color(BLACK - us);
  const auto mt = m.type();

  const Bitboard from_bb = Bitboards::square_bb(from);
  const Bitboard to_bb = Bitboards::square_bb(to);
  // For king safety we only need the resulting occupancy and the opponent's
  // remaining attackers. Avoid copying and updating the complete piece arrays
  // for every pseudo-legal move.
  Bitboard occ_all = (allPieces & ~from_bb) | to_bb;
  Bitboard captured_bb = to_bb;

  if (mt != MoveType::NORMAL) [[unlikely]] {
    switch (mt) {
      case MoveType::EN_PASSANT: {
        const Square capturedSq =
            (us == WHITE) ? Bitboards::down(to) : Bitboards::up(to);
        const Bitboard capturedSq_bb = Bitboards::square_bb(capturedSq);
        captured_bb |= capturedSq_bb;
        occ_all &= ~capturedSq_bb;
        break;
      }
      case MoveType::CASTLING: {
        Square rookFrom;
        Square rookTo;
        if (to > from) {  // kingside
          rookFrom = (us == WHITE) ? Square::H1 : Square::H8;
          rookTo = (us == WHITE) ? Square::F1 : Square::F8;
        } else {  // queenside
          rookFrom = (us == WHITE) ? Square::A1 : Square::A8;
          rookTo = (us == WHITE) ? Square::D1 : Square::D8;
        }
        const Bitboard r_bb =
            (Bitboards::square_bb(rookFrom) | Bitboards::square_bb(rookTo));
        occ_all ^= r_bb;
        break;
      }
      default:
        break;
    }
  }

  const Square king_sq =
      (pieces[us][KING] & from_bb) ? to : kingSq[us];
  const Bitboard remaining_enemy = ~captured_bb;

  // Bishop/Queen diagonal
  if (((pieces[them][BISHOP] | pieces[them][QUEEN]) & remaining_enemy) &
      Bitboards::bishop_attacks(king_sq, occ_all))
    return false;

  // Rook/Queen straight
  if (((pieces[them][ROOK] | pieces[them][QUEEN]) & remaining_enemy) &
      Bitboards::rook_attacks(king_sq, occ_all))
    return false;

  // Knight attackers
  if ((pieces[them][KNIGHT] & remaining_enemy) &
      Bitboards::knight_attacks(king_sq))
    return false;

  // Pawn attackers
  if ((pieces[them][PAWN] & remaining_enemy) &
      Bitboards::pawn_attacks_mask(king_sq, us))
    return false;

  if ((pieces[them][KING] & remaining_enemy) &
      Bitboards::king_attacks(king_sq))
    return false;

  return true;
}
