// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "board.h"
#include "fen.h"
#include "helpers.h"
#include "magic.h"
#include "movegen.h"
#include "zobrist.h"
#include "perft.h"

#ifdef DEBUG
uint64_t captures = 0;
uint64_t checks = 0;
#endif

// Usage: perft <depth> [FEN...] [-t threads]
// Runs single-threaded unless -t asks for more.
int main(int argc, char** argv) {
  Bitboards::init_magic_tables();
  Zobrist::init_zobrist_keys();
  int depth = 3;
  unsigned threads = 1;
  std::string selectedFEN;
  std::vector<std::string> positional;
  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "-t" && i + 1 < argc) {
      threads = static_cast<unsigned>(std::max(1, std::stoi(argv[++i])));
    } else {
      positional.push_back(arg);
    }
  }
  if (!positional.empty()) depth = std::stoi(positional[0]);
  for (size_t i = 1; i < positional.size(); ++i) {
    if (i > 1) selectedFEN += " ";
    selectedFEN += positional[i];
  }
  if (selectedFEN.empty())
    selectedFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -";
  if (depth < 0) {
    std::cerr << "Depth must be nonnegative\n";
    return 1;
  }
#ifdef DEBUG
  threads = 1;
#endif
  Board board = FEN::parse(selectedFEN);

  const auto start = std::chrono::steady_clock::now();
  uint64_t totalNodes = 0;

  if (depth == 0) {
    totalNodes = 1;
  } else {
    std::array<Move, MAX_MOVES> rootMoves;
    size_t nRootMoves = MoveGen::generate_all(board, rootMoves);

#ifdef DEBUG
    for (size_t i = 0; i < nRootMoves; ++i) {
      Move move = rootMoves[i];
      Board copy = board;
      copy.makeMove(move);
      uint64_t capturesBefore = captures;
      uint64_t checksBefore = checks;
      if (depth == 1) {
        captures += (copy.captured_piece != NO_PIECE);
        checks += copy.is_in_check(copy.sideToMove);
      }
      uint64_t nodes = perft<false>(copy, depth - 1, [](const Board& leaf) {
        captures += (leaf.captured_piece != NO_PIECE);
        checks += leaf.is_in_check(leaf.sideToMove);
      });
      totalNodes += nodes;
      std::cout << std::left << std::setw(4) << PrintingHelpers::move_to_str(move)
                << ": " << nodes << " (captures: " << captures - capturesBefore
                << ", checks: " << checks - checksBefore << ")\n";
    }
#else
    std::vector<uint64_t> counts(nRootMoves);
    std::atomic<size_t> next{0};
    auto worker = [&] {
      for (;;) {
        const size_t i = next.fetch_add(1, std::memory_order_relaxed);
        if (i >= nRootMoves) return;
        Board child = board;
        child.makeMove(rootMoves[i]);
        counts[i] = perft(child, depth - 1);
      }
    };
    std::vector<std::thread> workers;
    const unsigned workerCount = std::min(threads, static_cast<unsigned>(nRootMoves));
    for (unsigned i = 1; i < workerCount; ++i) workers.emplace_back(worker);
    worker();
    for (auto& thread : workers) thread.join();
    for (size_t i = 0; i < nRootMoves; ++i) {
      totalNodes += counts[i];
      std::cout << std::left << std::setw(4)
                << PrintingHelpers::move_to_str(rootMoves[i]) << ": "
                << counts[i] << '\n';
    }
#endif
  }

  const double seconds =
      std::chrono::duration<double>(std::chrono::steady_clock::now() - start)
          .count();
  std::cout << "\nTotal nodes: " << totalNodes << "\n";
  std::cout << "Time: " << std::fixed << std::setprecision(3) << seconds
            << " s, " << std::setprecision(1)
            << static_cast<double>(totalNodes) / seconds / 1e6 << " Mnps ("
            << threads << (threads == 1 ? " thread)\n" : " threads)\n");

#ifdef DEBUG
  std::cout << "Total captures: " << captures << "\n";
  std::cout << "Total checks: " << checks << "\n";
#endif

  return 0;
}
