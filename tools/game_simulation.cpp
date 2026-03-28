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
    if (n == 0 || __builtin_popcountl(board.allPieces) < 3) break;

    FoChess::g_search_stats.node_count = 0;
#ifdef DEBUG
    tt.hits = 0;
#endif
    auto start = std::chrono::high_resolution_clock::now();
    FoChess::iterative_deepening(6, board, tt);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    board.makeMove(FoChess::g_search_stats.best_move.load());

#ifdef DEBUG
    std::cout << "tt hits: " << tt.hits << "\n";
    tt.hits = 0;
#endif
    std::cout << "best move score: " << FoChess::g_search_stats.best_root_score.load()
              << " nodes/sec: " << FoChess::g_search_stats.nps(FoChess::g_search_state) << "\n";

    PrintingHelpers::printBoard(board);
  }

  return 0;
}
