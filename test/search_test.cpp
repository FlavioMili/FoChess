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
  Board board = FEN::parse("r1bq1rk1/ppp2ppp/2n2n2/2bpp1B1/2P5/3P1NP1/PP2PPBP/RN1Q1RK1 b - - 5 7");

  auto start = std::chrono::high_resolution_clock::now();
  SearchResult result = FoChess::alpha_beta_pruning(7, board, tt);
  auto end = std::chrono::high_resolution_clock::now();

  board.makeMove(result.move);

  std::chrono::duration<double> elapsed = end - start;
#ifdef DEBUG
    std::cout << "tt hits: " << tt.hits << "\n";
    tt.hits = 0;
#endif
    std::cout << "best move and score: " << PrintingHelpers::move_to_str(result.move) << result.score
              << " nodes/sec: " << static_cast<double>(FoChess::nodes) / elapsed.count() << "\n";
}
