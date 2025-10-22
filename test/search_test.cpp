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
      "5k1r/1pR1Q1pp/5p2/1p1Rp3/1P1pP2P/r2P1N2/3B1PP1/6K1 b - - 0 31"
  );

  auto start = std::chrono::high_resolution_clock::now();
  auto result = FoChess::iterative_deepening(8, board, tt);
  auto end = std::chrono::high_resolution_clock::now();

  board.makeMove(result.move);

  std::chrono::duration<double> elapsed = end - start;
#ifdef DEBUG
  std::cout << "tt hits: " << tt.hits << "\n";
  tt.hits = 0;
#endif
  std::cout << "best move and score: "
            << PrintingHelpers::move_to_str(result.move) << ", " << result.score
            << "\nnodes/sec: "
            << static_cast<double>(FoChess::nodes) / elapsed.count() << "\n";
}
