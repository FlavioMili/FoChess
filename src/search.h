// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#pragma once
#include <array>
#include <atomic>
#include <chrono>
#include <climits>
#include <cstdint>

#include "board.h"
#include "move.h"
#include "movegen.h"
#include "tt.h"

namespace FoChess {

constexpr int MATE_SCORE = 10000000;
constexpr int INF_SCORE = 2 * MATE_SCORE;

struct SearchState {
  std::atomic<bool> searching{false};
  std::chrono::steady_clock::time_point search_start;
  std::atomic<int64_t> time_limit{0};
  std::atomic<bool> should_stop{false};
};

struct SearchStatistics {
  std::atomic<uint64_t> node_count{0};
  std::atomic<int> highest_depth{0};
  std::atomic<int> best_root_score{-INF_SCORE};
  std::atomic<Move> best_move{Move::none()};

  int64_t elapsed_ms(const SearchState& state) const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now() - state.search_start)
        .count();
  }

  double nps(const SearchState& state) const {
    int64_t time_ms = elapsed_ms(state);
    uint64_t nodes = node_count.load(std::memory_order_relaxed);
    return time_ms > 0 ? (static_cast<double>(nodes) * 1000.0) /
                             static_cast<double>(time_ms)
                       : 0.0;
  }
};

inline SearchState g_search_state;
inline SearchStatistics g_search_stats;

void reset_search();
void reset_search_heuristics();
void end_search();
bool should_stop_search();

int bland_evaluate(const Board& board);

// Version without TT
int alpha_beta_pruning(int depth, Board& board, int alpha = -INF_SCORE,
                       int beta = INF_SCORE, int ply = 0);

// Version with TT
int alpha_beta_pruning(int depth, Board& board, TranspositionTable& tt,
                       int alpha = -INF_SCORE, int beta = INF_SCORE,
                       int ply = 0);

void iterative_deepening(int max_depth, Board& board, TranspositionTable& tt,
                         bool initialize = true);

int quiescence_search(Board& board, int alpha, int beta, int ply = 0);
int quiescence_search(Board& board, TranspositionTable& tt, int alpha,
                      int beta, int ply = 0);

}  // namespace FoChess

inline void FoChess::reset_search() {
  reset_search_heuristics();
  // State
  g_search_state.searching.store(true, std::memory_order_relaxed);
  g_search_state.search_start = std::chrono::steady_clock::now();
  // g_search_state.time_limit.store(0, std::memory_order_relaxed);
  g_search_state.should_stop.store(false, std::memory_order_relaxed);

  // Statistics
  g_search_stats.node_count.store(0, std::memory_order_relaxed);
  g_search_stats.highest_depth.store(0, std::memory_order_relaxed);
  g_search_stats.best_root_score.store(INT_MIN+ 1, std::memory_order_relaxed);
  g_search_stats.best_move.store(Move::none(), std::memory_order_relaxed);
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
                                         TranspositionTable& tt,
                                         bool initialize) {
  if (initialize) reset_search();

  // Always retain a legal fallback in case the first iteration is interrupted.
  std::array<Move, MAX_MOVES> root_moves;
  const size_t root_count = MoveGen::generate_all(board, root_moves);
  if (root_count > 0) {
    g_search_stats.best_move.store(root_moves[0], std::memory_order_relaxed);
    g_search_stats.best_root_score.store(bland_evaluate(board),
                                         std::memory_order_relaxed);
  }

  for (int depth = 1; depth <= max_depth; ++depth) {
    if (should_stop_search()) break;

    const Move completed_move =
        g_search_stats.best_move.load(std::memory_order_relaxed);
    const int completed_score =
        g_search_stats.best_root_score.load(std::memory_order_relaxed);

    int score = alpha_beta_pruning(depth, board, tt, -INF_SCORE, INF_SCORE);

    if (should_stop_search()) {
      g_search_stats.best_move.store(completed_move,
                                     std::memory_order_relaxed);
      g_search_stats.best_root_score.store(completed_score,
                                           std::memory_order_relaxed);
      break;
    }

    g_search_stats.highest_depth.store(depth, std::memory_order_relaxed);
    g_search_stats.best_root_score.store(score, std::memory_order_relaxed);
  }

  end_search();
}
