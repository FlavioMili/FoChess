#include <array>
#include <cstddef>
#include <iostream>

#include "board.h"
#include "fen.h"
#include "movegen.h"

size_t perft(int depth, Board& board) {
  if (depth == 0) return 1;

  std::array<Move, MAX_MOVES> moves;

  size_t nodes = 0;
  size_t n_moves = MoveGen::generate_all(board, moves);

  if (depth == 1) return moves.size();

  for (size_t i = 0; i < n_moves; ++i) {
    Board copy = board;
    copy.makeMove(moves[i]);
    nodes += perft(depth - 1, copy);
  }

  return nodes;
}

int main() {
  std::string startFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -";
  Board board = FEN::parse(startFEN);
  size_t nodes = perft(7, board);
  std::cout << "nodes: " << nodes << "\n";
  return 0;
}
