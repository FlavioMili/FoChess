#include <cctype>
#include <iostream>
#include "board.h"
#include "movegen.h"
#include "types.h"

namespace PrintingHelpers {

constexpr char pieceChar(Piece pt, Color c) {
  char ch = '?';
  switch (pt) {
    case PAWN:
      ch = 'P';
      break;
    case KNIGHT:
      ch = 'N';
      break;
    case BISHOP:
      ch = 'B';
      break;
    case ROOK:
      ch = 'R';
      break;
    case QUEEN:
      ch = 'Q';
      break;
    case KING:
      ch = 'K';
      break;
    default:
      ch = '-';
      break;
  }
  if (c == BLACK) ch = static_cast<char>(std::tolower(ch));
  return ch;
}

inline void printBoard(Board board) {
  for (int rank = 7; rank >= 0; --rank) {
    std::cout << rank + 1 << "  ";
    for (int file = 0; file < 8; ++file) {
      int sq = rank * 8 +  (7 - file);  // A8=0, H1=63
      Bitboard mask = 1ULL << sq;

      char symbol = '.';
      for (size_t c = 0; c < 2; ++c) {
        for (size_t p = 0; p < 6; ++p) {
          if (board.pieces[c][p] & mask) {
            symbol = pieceChar(static_cast<Piece>(p), static_cast<Color>(c));
            goto print_done;
          }
        }
      }
    print_done:
      std::cout << symbol << ' ';
    }
    std::cout << '\n';
  }
  std::cout << "\n   a b c d e f g h\n";
}

inline std::string square_to_str(Square sq) {
  char file = 'a' + (sq % 8);
  char rank = static_cast<char>('1' + (7 - (sq / 8)));  // rank 1 is bottom
  return {file, rank};
}

inline std::string move_to_str(const Move& m, const Board& board) {
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
      case QUEEN:
        s += "=Q";
        break;
      case ROOK:
        s += "=R";
        break;
      case BISHOP:
        s += "=B";
        break;
      case KNIGHT:
        s += "=N";
        break;
      default:
        break;
    }
  }

  return s;
}

} // namespace PrintingHelpers

