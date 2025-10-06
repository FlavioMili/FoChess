#include <array>
#include <cstddef>
#include <iostream>
#include <string>

#include "board.h"
#include "fen.h"
#include "movegen.h"

#ifdef DEBUG
int captures = 0;
#endif

uint64_t perft(Board& board, int depth) {
    if (depth == 0) return 1;

    std::array<Move, MAX_MOVES> moves;
    size_t n = MoveGen::generate_all(board, moves);
    uint64_t nodes = 0;

    for (size_t i = 0; i < n; ++i) {
        Move move = moves[i];
        Board copy = board;
        copy.makeMove(move);

#ifdef DEBUG
    captures+= copy.was_captured;
#endif

        nodes += perft(copy, depth - 1);
    }
    return nodes;
}

int main(int argc, char** argv) {
  int n = argc > 1 ? std::stoi(argv[1]) : 3;
  std::string startFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -";
  Board board = FEN::parse(startFEN);
  size_t nodes = perft(board, n);
  std::cout << "nodes: " << nodes << "\n";
#ifdef DEBUG
 std::cout << "number of captures: " << captures << "\n";
#endif
  return 0;
}
