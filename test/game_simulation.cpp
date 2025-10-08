#include <array>

#include "board.h"
#include "fen.h"
#include "helpers.h"
#include "move.h"
#include "movegen.h"

int main() {
  Board board = FEN::parse("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -");
  std::array<Move, MAX_MOVES> moves;
  int n = MoveGen::generate_all(board, moves);
  while (n > 0) {
    board.makeMove(moves[0]);
    PrintingHelpers::printBoard(board);
    n = MoveGen::generate_all(board, moves);
  }

  return 0;
}
