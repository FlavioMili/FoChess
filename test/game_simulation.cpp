// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#include <array>
#include <chrono>
#include <cstddef>
#include <iostream>

#include "board.h"
#include "fen.h"
#include "helpers.h"
#include "move.h"
#include "movegen.h"
#include "search.h"
#include "zobrist.h"

// this test is thought for a bland simulation and just test performance, not actual functionality
int main() {
  Board board = FEN::parse();

  Zobrist::init_zobrist_keys();
  TranspositionTable tt;

  while (true) {
    std::array<Move, MAX_MOVES> moves;
    size_t n = MoveGen::generate_all(board, moves);
    // if (n == 0 || __builtin_popcountl(board.allPieces) < 3) break;

    FoChess::nodes = 0;
#ifdef DEBUG
    tt.hits = 0;
#endif
    auto start = std::chrono::high_resolution_clock::now();
    SearchResult result = FoChess::alpha_beta_pruning(7, board, tt);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    board.makeMove(result.move);

#ifdef DEBUG
    std::cout << "tt hits: " << tt.hits << "\n";
    tt.hits = 0;
#endif
    // std::cout << "best move score: " << result.score
    //           << " nodes/sec: " << static_cast<double>(FoChess::nodes) / elapsed.count() << "\n";
    //
    // PrintingHelpers::printBoard(board);
  }

  return 0;
}
