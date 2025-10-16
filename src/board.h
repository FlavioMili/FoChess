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
#include "magic.h"

// I made some tests with an enum : uint8_t 
// but this implementation was much faster 
// apparently so I will keep it like this
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

inline bool is_in_check(Color c) const noexcept __attribute__((always_inline));

  Color color_on(Square sq) const;
  Piece piece_on(Square sq) const;

  void makeMove(const Move& m);
  // TODO implement
  // bool isLegalMove(const Move& m) const;

  std::array<std::array<Bitboard, 6>, 2> pieces;  // [color][pieceType]
  std::array<Bitboard, 2> occupancy;              // white/black
  // std::array<Piece, 64> square_to_piece; TODO implement
  Bitboard allPieces;                             // all occupied squares
  CastlingRights castling;  // castling rights flags
  uint16_t fullMoveNumber;
  uint8_t halfMoveClock;
  Square enPassant = NO_SQUARE;  // en passant target
  Color sideToMove;
  Piece captured_piece;
};

inline void Board::updateOccupancy() {
  occupancy[WHITE] = pieces[WHITE][PAWN] | pieces[WHITE][KNIGHT] | pieces[WHITE][BISHOP] |
                     pieces[WHITE][ROOK] | pieces[WHITE][QUEEN] | pieces[WHITE][KING];

  occupancy[BLACK] = pieces[BLACK][PAWN] | pieces[BLACK][KNIGHT] | pieces[BLACK][BISHOP] |
                     pieces[BLACK][ROOK] | pieces[BLACK][QUEEN] | pieces[BLACK][KING];

  allPieces = occupancy[WHITE] | occupancy[BLACK];
}

inline Bitboard Board::attacks_to(Square sq, Color attacker_color) const {
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

inline bool Board::is_in_check(Color c) const noexcept {
  Bitboard king_bb = pieces[c][KING];

  Square kingSquare = static_cast<Square>(__builtin_ctzll(king_bb));
  Color enemy = Color(BLACK - c);

  return attacks_to(kingSquare, enemy) != 0;
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
