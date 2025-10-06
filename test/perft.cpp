#include <array>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <string>

#include "board.h"
#include "fen.h"
#include "helpers.h"
#include "movegen.h"

#ifdef DEBUG
int captures = 0;
#endif

size_t perft(Board& board, int depth) {
  if (depth == 0) return 1;

  std::array<Move, MAX_MOVES> moves;
  size_t n = MoveGen::generate_all(board, moves);
  uint64_t nodes = 0;

  for (size_t i = 0; i < n; ++i) {
    Move move = moves[i];
    Board copy = board;
    copy.makeMove(move);

#ifdef DEBUG
    captures += copy.was_captured;
#endif

    nodes += perft(copy, depth - 1);
  }

  return nodes;
}

int main(int argc, char** argv) {
  int depth = argc > 1 ? std::stoi(argv[1]) : 3;
  std::string startFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -";
  Board board = FEN::parse(startFEN);

  // Generate root moves once
  std::array<Move, MAX_MOVES> rootMoves;
  size_t nRootMoves = MoveGen::generate_all(board, rootMoves);

  uint64_t totalNodes = 0;

  for (size_t i = 0; i < nRootMoves; ++i) {
    Move move = rootMoves[i];
    Board copy = board;
    copy.makeMove(move);

#ifdef DEBUG
    int capturesBefore = captures;
#endif

    // Count nodes under this move
    uint64_t nodes = perft(copy, depth - 1);
    totalNodes += nodes;

    // Print per-root-move count
    std::cout << std::left << std::setw(6) << PrintingHelpers::move_to_str(move) << ": " << nodes;

#ifdef DEBUG
    std::cout << " (captures: " << captures - capturesBefore << ")";
#endif

    std::cout << '\n';
  }

  std::cout << "\nTotal nodes: " << totalNodes << "\n";
#ifdef DEBUG
  std::cout << "Total captures: " << captures << "\n";
#endif

  return 0;
}
