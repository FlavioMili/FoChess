// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#pragma once

#include <atomic>
#include <chrono>
#include <climits>
#include <cstdint>
#include <cstdlib>

#include "board.h"
#include "move.h"
#include "tt.h"

struct SearchResult {
  int score;
  Move move;
};

namespace FoChess {

int bland_evaluate(const Board& board);
bool should_stop_search();

// Version without TT
SearchResult alpha_beta_pruning(uint8_t depth, Board& board,
                                int alpha = INT_MIN + 1,
                                int beta = INT_MAX - 1);

// Version with TT
SearchResult alpha_beta_pruning(uint8_t depth, Board& board,
                                TranspositionTable& tt, int alpha = INT_MIN + 1,
                                int beta = INT_MAX - 1);

SearchResult iterative_deepening(uint8_t max_depth, Board& board,
                                 TranspositionTable& tt);

SearchResult quiescence_search(Board& board, TranspositionTable& tt, int alpha,
                               int beta);

inline int nodes = 0;
constexpr int MATE_SCORE = 10000000;

inline std::atomic<bool> g_should_stop{false};
inline std::chrono::steady_clock::time_point g_search_start;
inline std::atomic<int64_t> g_time_limit{0};

}  // namespace FoChess

inline bool FoChess::should_stop_search() {
  if (g_should_stop.load()) return true;

  // Check time limit if set
  if (g_time_limit.load() > 0) {
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                       std::chrono::steady_clock::now() - g_search_start)
                       .count();

    if (elapsed >= g_time_limit.load()) return true;
  }

  return false;
}

inline SearchResult FoChess::iterative_deepening(uint8_t max_depth,
                                                 Board& board,
                                                 TranspositionTable& tt) {
  SearchResult best_result = {INT_MIN + 1, Move()};

  for (uint8_t depth = 1; depth <= max_depth; ++depth) {
    // Check if we should stop before starting new iteration
    if (should_stop_search()) break;

    SearchResult current = alpha_beta_pruning(depth, board, tt);

    if (!should_stop_search() && current.move != Move()) best_result = current;
    else break;

    if (std::abs(best_result.score) >= MATE_SCORE - 1000) break;
  }

  return best_result;
}
