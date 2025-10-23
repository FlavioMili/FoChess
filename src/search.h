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

#include "board.h"
#include "move.h"
#include "tt.h"

struct SearchState {
  std::atomic<bool> searching{false};
  std::chrono::steady_clock::time_point search_start;
  std::atomic<int64_t> time_limit{0};
  std::atomic<bool> should_stop{false};
};

struct SearchStatistics {
  std::atomic<uint64_t> node_count{0};
  std::atomic<int> highest_depth{0};
  std::atomic<int> best_root_score{INT_MIN + 1};
  std::atomic<Move> best_move{Move{}};

  int64_t elapsed_ms(const SearchState& state) const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now() - state.search_start)
        .count();
  }

  double nps(const SearchState& state) const {
    int64_t time_ms = elapsed_ms(state);
    uint64_t nodes = node_count.load(std::memory_order_relaxed);
    return time_ms > 0 ? (nodes * 1000.0) / time_ms : 0.0;
  }
};

namespace FoChess {

inline SearchState g_search_state;
inline SearchStatistics g_search_stats;

constexpr int MATE_SCORE = 10000000;

void reset_search();
void end_search();
bool should_stop_search();

int bland_evaluate(const Board& board);

// Version without TT
int alpha_beta_pruning(int depth, Board& board, int alpha = INT_MIN + 1,
                       int beta = INT_MAX - 1, int ply = 0);

// Version with TT
// int alpha_beta_pruning(int depth, Board& board, TranspositionTable& tt,
//                        int alpha = INT_MIN + 1, int beta = INT_MAX - 1);

void iterative_deepening(int max_depth, Board& board, TranspositionTable& tt);

int quiescence_search(Board& board, int alpha,
                      int beta);

}  // namespace FoChess

inline void FoChess::reset_search() {
  // State
  g_search_state.searching.store(true, std::memory_order_relaxed);
  g_search_state.search_start = std::chrono::steady_clock::now();
  // g_search_state.time_limit.store(0, std::memory_order_relaxed);
  g_search_state.should_stop.store(false, std::memory_order_relaxed);

  // Statistics
  g_search_stats.node_count.store(0, std::memory_order_relaxed);
  g_search_stats.highest_depth.store(0, std::memory_order_relaxed);
  g_search_stats.best_root_score.store(INT_MIN + 1, std::memory_order_relaxed);
  g_search_stats.best_move.store(Move{}, std::memory_order_relaxed);
}

inline void FoChess::end_search() {
  g_search_state.searching.store(false, std::memory_order_relaxed);
}

inline bool FoChess::should_stop_search() {
  if (g_search_state.should_stop.load(std::memory_order_relaxed)) return true;

  // Check time limit if set
  if (g_search_state.time_limit.load(std::memory_order_relaxed) > 0) {
    auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - g_search_state.search_start)
            .count();
    if (elapsed >= g_search_state.time_limit.load(std::memory_order_relaxed)) {
      g_search_state.should_stop.store(true, std::memory_order_relaxed);
      return true;
    }
  }
  return false;
}

inline void FoChess::iterative_deepening(int max_depth, Board& board,
                                        TranspositionTable& tt) {
  reset_search();
  
  for (int depth = 1; depth <= max_depth; ++depth) {
    if (should_stop_search()) break;
    
    int score = alpha_beta_pruning(depth, board, INT_MIN + 1, INT_MAX - 1, 0);
    
    if (should_stop_search()) break;
    
    // Update statistics - use .store() for atomics!
    g_search_stats.highest_depth.store(depth, std::memory_order_relaxed);
    g_search_stats.best_root_score.store(score, std::memory_order_relaxed);
  }
  
  end_search();
}
