// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#pragma once

#include <cstdint>

enum Square : uint8_t {
  A8, B8, C8, D8, E8, F8, G8, H8,
  A7, B7, C7, D7, E7, F7, G7, H7,
  A6, B6, C6, D6, E6, F6, G6, H6,
  A5, B5, C5, D5, E5, F5, G5, H5,
  A4, B4, C4, D4, E4, F4, G4, H4,
  A3, B3, C3, D3, E3, F3, G3, H3,
  A2, B2, C2, D2, E2, F2, G2, H2,
  A1, B1, C1, D1, E1, F1, G1, H1,

  NO_SQUARE
};

enum Piece : uint8_t { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, NO_PIECE };

enum Color : uint8_t { WHITE, BLACK, NO_COLOR };

/**
 * This is initialised to empty squares so that the generation
 * of boards is simplified.
 */
struct SquareInfo {
  Piece type = NO_PIECE;
  Color color = NO_COLOR;
};

enum MoveType : uint8_t { 
  NORMAL = 0,
  PROMOTION = 1 << 2,
  EN_PASSANT = 2 << 2,
  CASTLING = 3 << 2
};

// This class was originally very similar to the one Stockfish implemented
// After implementing mine I looked at Stockfish code and changed
// a few things (like inverting from and to inside the move bits)
// I then renamed the methods so that whoever comes from Stockfish
// could more easily understand what they are reading.
//
// Also, small thing I noticed, in Stockfish there is this constructor
//
// constexpr Move(Square from, Square to) :
//     data((from << 6) + to) {}
// And I thought that using an | (or) instead of + would make more sense but
// this is what godbolt.org shows as assembly
//
// func1(unsigned char, unsigned char):   ///////////// Function using |
//         movzx   edi, dil
//         movzx   esi, sil
//         sal     edi, 6
//         mov     eax, edi
//         or      eax, esi
//         ret
// func2(unsigned char, unsigned char):   ///////////// Function using +
//         movzx   eax, dil
//         lea     ecx, [rsi+6]
//         sal     eax, cl
//         ret
//

/*
 * Move Class
*/
class Move {
 public:
  constexpr Move() = default;
  constexpr Move(Square from, Square to) :
    move(static_cast<uint16_t>((from << 6) + to)) {}

  constexpr Square from_sq() const { return Square((move >> 6) & 0x3F); }
  constexpr Square to_sq() const { return Square(move & 0x3F); }

  constexpr std::uint16_t raw() const { return move; }

  constexpr bool operator==(const Move& m) const { return move == m.move; }
  constexpr bool operator!=(const Move& m) const { return move != m.move; }

  static constexpr Move make(Square from, Square to, Piece pt = KNIGHT, MoveType type = MoveType::NORMAL) {
    return Move(static_cast<uint16_t>(type | ((pt - KNIGHT) << 12) | (from << 6) | to));
  }

  constexpr MoveType type() const { return MoveType((move >> 14) & 3); }
  constexpr Piece promotion_type() const { return Piece(((move >> 12) & 3) + KNIGHT); }

 protected:
  constexpr explicit Move(std::uint16_t m) : move(m) {}
  std::uint16_t move = 0;
};
