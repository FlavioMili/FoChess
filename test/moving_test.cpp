#include <string>
#include "board.h"
#include "fen.h"
#include "helpers.h"
#include "types.h"

int main () {
  std::string startFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -";
  Board board = FEN::parse(startFEN);

  // print before
  PrintingHelpers::printBoard(board);

  board.makeMove(Move(Square::E2, Square::E3));

  // print after
  PrintingHelpers::printBoard(board);

  board.makeMove(Move(Square::E3, Square::E8));
  PrintingHelpers::printBoard(board);


  return 0;
}
