#include <cstddef>
#include <iostream>
#include <vector>

#include "board.h"
#include "fen.h"
#include "movegen.h"

size_t perft(int depth, Board& board) {
  if (depth == 0)
    return 1;

  size_t nodes = 0;
  std::vector<Move> moves = MoveGen::generate_all(board);

  if (depth == 1)
    return moves.size();

  for (const auto& move : moves) {
    Board copy = board;
    copy.makeMove(move);
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
