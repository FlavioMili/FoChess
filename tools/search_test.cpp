// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#include "search.h"

#include <chrono>
#include <iostream>

#include "board.h"
#include "fen.h"
#include "helpers.h"
#include "tt.h"
#include "zobrist.h"

int main() {
  Zobrist::init_zobrist_keys();
  TranspositionTable tt;
  Board board = FEN::parse(
      "5k1r/1pR1Q1pp/5p2/1p1Rp3/1P1pP2P/r2P1N2/3B1PP1/6K1 b - - 0 31");

  auto start = std::chrono::high_resolution_clock::now();
  FoChess::iterative_deepening(8, board, tt);
  auto end = std::chrono::high_resolution_clock::now();

  board.makeMove(FoChess::g_search_stats.best_move.load());

  std::chrono::duration<double> elapsed = end - start;
#ifdef DEBUG
  std::cout << "tt hits: " << tt.hits << "\n";
  tt.hits = 0;
#endif
  std::cout << "best move and score: "
            << PrintingHelpers::move_to_str(
                   FoChess::g_search_stats.best_move.load())
            << ", " << FoChess::g_search_stats.best_root_score.load()
            << "\nnodes/sec: "
            << FoChess::g_search_stats.nps(FoChess::g_search_state) << "\n";
}
