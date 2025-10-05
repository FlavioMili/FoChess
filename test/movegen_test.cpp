
#include "movegen.h"

#include <iostream>
#include <string>
#include <vector>

#include "board.h"
#include "fen.h"

std::string square_to_str(Square sq) {
  char file = 'a' + (sq % 8);
  char rank = static_cast<char>('1' + (7 - (sq / 8)));  // rank 1 is bottom
  return {file, rank};
}

std::string move_to_str(const Move& m, const Board& board) {
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

int main() {
  std::string startFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -";
  Board board = FEN::parse(startFEN);

  std::cout << "Printing parsed board:\n";
  board.printBoard();

  std::vector<Move> mg = MoveGen::generate_all(board);

  std::cout << "\nGenerated " << mg.size() << " moves:\n";
  for (const auto& move : mg) {
    std::cout << move_to_str(move, board) << "\n";
  }

  return 0;
}
