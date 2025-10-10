// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#include <cctype>
#include <iostream>

#include "board.h"
#include "types.h"

#pragma once

namespace PrintingHelpers {

constexpr char pieceChar(Piece pt, Color c) {
  char ch = '?';
  switch (pt) {
    case PAWN: ch = 'P'; break;
    case KNIGHT: ch = 'N'; break;
    case BISHOP: ch = 'B'; break;
    case ROOK: ch = 'R'; break;
    case QUEEN: ch = 'Q'; break;
    case KING: ch = 'K'; break;
    default: ch = '-'; break;
  }
  if (c == BLACK) ch = static_cast<char>(std::tolower(ch));
  return ch;
}

inline void printBoard(Board board) {
  std::cout << "\n";
  for (int rank = 7; rank >= 0; --rank) {
    std::cout << rank + 1 << "  ";
    for (int file = 0; file < 8; ++file) {
      int sq = (7 - rank) * 8 + file;
      Bitboard mask = 1ULL << sq;
      char symbol = '.';

      for (size_t c = 0; c < 2 && symbol == '.'; ++c) {
        for (size_t p = 0; p < 6 && symbol == '.'; ++p) {
          if (board.pieces[c][p] & mask) {
            symbol = pieceChar(static_cast<Piece>(p), static_cast<Color>(c));
          }
        }
      }

      std::cout << symbol << ' ';
    }
    std::cout << '\n';
  }
  std::cout << "\n   a b c d e f g h\n";
}

inline std::string square_to_str(Square sq) {
  if (sq == Square::NO_SQUARE) return "NO_SQUARE";
  int rank = 8 - (sq / 8);
  int file = sq % 8;
  return {char('a' + file), char('0' + rank)};
}

inline std::string nice_move_to_str(const Move& m, const Board& board) {
  Piece pt = board.piece_on(m.from_sq());
  Color c = board.color_on(m.from_sq());
  char pc = PrintingHelpers::pieceChar(pt, c);

  std::string s;
  s += pc;                          // piece char
  s += square_to_str(m.from_sq());  // from square
  s += '-';
  s += square_to_str(m.to_sq());  // to square

  if (m.type() == PROMOTION) {
    switch (m.promotion_type()) {
      case QUEEN: s += "=Q"; break;
      case ROOK: s += "=R"; break;
      case BISHOP: s += "=B"; break;
      case KNIGHT: s += "=N"; break;
      default: break;
    }
  }
  return s;
}


inline std::string move_to_str(const Move& m) {
  std::string s;
  s += square_to_str(m.from_sq());  // from square
  s += square_to_str(m.to_sq());  // to square

  if (m.type() == PROMOTION) {
    switch (m.promotion_type()) {
      case QUEEN: s += "q"; break;
      case ROOK: s += "r"; break;
      case BISHOP: s += "b"; break;
      case KNIGHT: s += "n"; break;
      default: break;
    }
  }
  return s;
}

inline Move uci_to_move(const std::string& s) {
  if (s.size() < 4) return Move(); // invalid

  // Convert file+rank to Square
  int from_file = s[0] - 'a';
  int from_rank = '8' - s[1]; // rank 8 -> 0
  int to_file   = s[2] - 'a';
  int to_rank   = '8' - s[3];

  Square from_sq = static_cast<Square>(from_rank * 8 + from_file);
  Square to_sq   = static_cast<Square>(to_rank * 8 + to_file);

  // Check for promotion
  if (s.size() == 5) {
    Piece promo = QUEEN;
    switch (s[4]) {
      case 'q': promo = QUEEN; break;
      case 'r': promo = ROOK;  break;
      case 'b': promo = BISHOP; break;
      case 'n': promo = KNIGHT; break;
    }
    return Move(from_sq, to_sq, promo);
  }
  return Move(from_sq, to_sq);
}

}  // namespace PrintingHelpers
