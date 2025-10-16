// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#include <array>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <string>

#include "board.h"
#include "fen.h"
#include "helpers.h"
#include "magic.h"
#include "movegen.h"
#include "types.h"

struct MagicInitializer {
  MagicInitializer() { Bitboards::init_magic_tables(); }
};
static MagicInitializer magic_initializer;

struct PerftResult {
  uint64_t nodes = 0;
  int captures = 0;
  int checks = 0;
};

PerftResult perft(Board& board, int depth) {
  if (depth == 0) {
    return {1, 0, 0};
  }

  std::array<Move, MAX_MOVES> moves;
  size_t n = MoveGen::generate_all(board, moves);

  PerftResult result;
  for (size_t i = 0; i < n; ++i) {
    Move move = moves[i];
    Board copy = board;
    copy.makeMove(move);

    PerftResult branch_result = perft(copy, depth - 1);
    result.nodes += branch_result.nodes;
    result.captures += branch_result.captures;
    result.checks += branch_result.checks;

    if (depth == 1) {
      if (copy.captured_piece != NO_PIECE) result.captures++;
      if (copy.is_in_check(copy.sideToMove)) result.checks++;
    }
  }

  return result;
}

int main(int argc, char** argv) {
  int depth = argc > 1 ? std::stoi(argv[1]) : 3;
  std::string startFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -";
  std::string secondFEN = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ";
  std::string selectedFEN = argc > 2 ? argv[2] : startFEN;
  Board board = FEN::parse(selectedFEN);

  // Generate root moves once
  std::array<Move, MAX_MOVES> rootMoves;
  size_t nRootMoves = MoveGen::generate_all(board, rootMoves);

  PerftResult total_result;

  for (size_t i = 0; i < nRootMoves; ++i) {
    Move move = rootMoves[i];
    Board copy = board;
    copy.makeMove(move);

    PerftResult branch_result = perft(copy, depth - 1);
    total_result.nodes += branch_result.nodes;
    total_result.captures += branch_result.captures;
    total_result.checks += branch_result.checks;

    // Print per-root-move count
    std::cout << std::left << std::setw(6) << PrintingHelpers::move_to_str(move) << ": " << branch_result.nodes;
    std::cout << " (captures: " << branch_result.captures << ", checks: " << branch_result.checks << ")";
    std::cout << '\n';
  }

  std::cout << "\nTotal nodes: " << total_result.nodes << "\n";
  std::cout << "Total captures: " << total_result.captures << "\n";
  std::cout << "Total checks: " << total_result.checks << "\n";

  return 0;
}
