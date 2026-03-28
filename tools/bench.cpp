// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#include <chrono>
#include <fstream>
#include <iostream>
#include <string>

#include "fen.h"
#include "helpers.h"
#include "magic.h"
#include "search.h"
#include "tt.h"
#include "zobrist.h"

int main() {
  std::ifstream fenFile("resources/bench_fen_list.txt");
  if (!fenFile) {
    std::cerr << "Failed to open FEN file.\n";
    return 1;
  }
  Zobrist::init_zobrist_keys();
  Bitboards::init_magic_tables();
  TranspositionTable tt;

  std::string line;
  int posCount = 0;

  while (std::getline(fenFile, line)) {
    if (line.empty() || line[0] == '#') continue;
    ++posCount;

    Board board = FEN::parse(line);

    auto start = std::chrono::high_resolution_clock::now();

    FoChess::iterative_deepening(5, board, tt);

    // Find best move (assuming you have this function)
    Move best = FoChess::g_search_stats.best_move.load();

    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "Position " << posCount << ":\n";
    std::cout << "FEN: " << line << "\n";
    std::cout << "Seen nodes: " << FoChess::g_search_stats.node_count << "\n";
    std::cout << "Best move: " << PrintingHelpers::move_to_str(best) << "\n";
    std::cout << "Time: " << ms << " ms\n\n";
  }

  return 0;
}
