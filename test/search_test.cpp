#include "search.h"
#include <chrono>
#include <iostream>
#include "board.h"
#include "fen.h"
#include "tt.h"
#include "zobrist.h"

int main() {
  Zobrist::init_zobrist_keys();
  TranspositionTable tt;
  Board board = FEN::parse();

  auto start = std::chrono::high_resolution_clock::now();
  SearchResult result = FoChess::alpha_beta_pruning(8, board, tt);
  auto end = std::chrono::high_resolution_clock::now();

  board.makeMove(result.move);

  std::chrono::duration<double> elapsed = end - start;
#ifdef DEBUG
    std::cout << "tt hits: " << tt.hits << "\n";
    tt.hits = 0;
#endif
    std::cout << "best move score: " << result.score
              << " nodes/sec: " << static_cast<double>(FoChess::nodes) / elapsed.count() << "\n";
}
