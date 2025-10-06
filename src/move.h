// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#pragma once

#include "types.h"

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
  constexpr Move(Square from, Square to, Piece pt = KNIGHT, MoveType type = MoveType::NORMAL)
      : move(static_cast<uint16_t>((type) | ((pt - KNIGHT) << 12) | (from << 6) | to)) {}

  constexpr Square from_sq() const { return Square((move >> 6) & 0x3F); }
  constexpr Square to_sq() const { return Square(move & 0x3F); }

  constexpr std::uint16_t raw() const { return move; }

  constexpr bool operator==(const Move& m) const { return move == m.move; }
  constexpr bool operator!=(const Move& m) const { return move != m.move; }

  constexpr MoveType type() const { return MoveType((move >> 14) & 3); }
  constexpr Piece promotion_type() const { return Piece(((move >> 12) & 3) + KNIGHT); }

 protected:
  constexpr explicit Move(std::uint16_t m) : move(m) {}
  std::uint16_t move = 0;
};

