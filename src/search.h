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
#include <span>

#include "board.h"
#include "move.h"
#include "movegen.h"
#include "tt.h"

namespace FoChess {

#ifdef SEARCH_DIAGNOSTICS
struct SearchDiagnostics {
  uint64_t main_nodes = 0;
  uint64_t quiescence_nodes = 0;
  uint64_t main_generated_moves = 0;
  uint64_t main_searched_moves = 0;
  uint64_t quiescence_generated_moves = 0;
  uint64_t quiescence_searched_moves = 0;
  uint64_t quiescence_recursive_moves = 0;
  uint64_t stand_pat_cutoffs = 0;
  uint64_t delta_prunes = 0;
  uint64_t quiescence_beta_cutoffs = 0;
  uint64_t quiescence_repeats = 0;
  uint64_t quiescence_tt_cutoffs = 0;
  uint64_t beta_cutoffs = 0;
  std::array<uint64_t, 6> cutoff_buckets{};
  uint64_t lmr_searches = 0;
  uint64_t lmr_researches = 0;
  uint64_t null_searches = 0;
  uint64_t null_cutoffs = 0;
  uint64_t tt_cutoffs = 0;
  uint64_t aspiration_searches = 0;
  uint64_t aspiration_researches = 0;
};

extern SearchDiagnostics g_search_diagnostics;
void reset_search_diagnostics();
#endif

constexpr int MATE_SCORE = 10000000;
constexpr int INF_SCORE = 2 * MATE_SCORE;

struct SearchState {
  std::atomic<bool> searching{false};
  std::chrono::steady_clock::time_point search_start;
  std::atomic<int64_t> soft_time_limit{0};
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
void prepare_repetition_history(uint64_t root_hash,
                                std::span<const uint64_t> history);

int bland_evaluate(const Board& board);
int static_exchange_evaluation(const Board& board, const Move& move);

// Version without TT
int alpha_beta_pruning(int depth, Board& board, int alpha = -INF_SCORE,
                       int beta = INF_SCORE, int ply = 0,
                       bool allow_null = true);

// Version with TT
int alpha_beta_pruning(int depth, Board& board, TranspositionTable& tt,
                       int alpha = -INF_SCORE, int beta = INF_SCORE,
                       int ply = 0, bool allow_null = true);

void iterative_deepening(int max_depth, Board& board, TranspositionTable& tt,
                         bool initialize = true,
                         std::span<const uint64_t> history = {});

int quiescence_search(Board& board, int alpha, int beta, int ply = 0);
int quiescence_search(Board& board, TranspositionTable& tt, int alpha,
                      int beta, int ply = 0);

}  // namespace FoChess

inline void FoChess::reset_search() {
  reset_search_heuristics();
  prepare_repetition_history(0, {});
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
                                         bool initialize,
                                         std::span<const uint64_t> history) {
  if (initialize) reset_search();
  prepare_repetition_history(board.hash, history);

  // Always retain a legal fallback in case the first iteration is interrupted.
  std::array<Move, MAX_MOVES> root_moves;
  const size_t root_count = MoveGen::generate_all(board, root_moves);
  if (root_count > 0) {
    g_search_stats.best_move.store(root_moves[0], std::memory_order_relaxed);
    g_search_stats.best_root_score.store(bland_evaluate(board),
                                         std::memory_order_relaxed);
  }

  Move previous_iteration_move = Move::none();
  int stable_iterations = 0;
  int previous_iteration_score = 0;

  for (int depth = 1; depth <= max_depth; ++depth) {
    if (should_stop_search()) break;

    const Move completed_move =
        g_search_stats.best_move.load(std::memory_order_relaxed);
    const int completed_score =
        g_search_stats.best_root_score.load(std::memory_order_relaxed);

    int score;
    if (depth >= 4 && previous_iteration_score > -MATE_SCORE + 256 &&
        previous_iteration_score < MATE_SCORE - 256) {
      int window = 100;
      int alpha = std::max(-INF_SCORE, previous_iteration_score - window);
      int beta = std::min(INF_SCORE, previous_iteration_score + window);

      while (true) {
#ifdef SEARCH_DIAGNOSTICS
        ++g_search_diagnostics.aspiration_searches;
#endif
        score = alpha_beta_pruning(depth, board, tt, alpha, beta);
        if (should_stop_search() || (score > alpha && score < beta)) break;

#ifdef SEARCH_DIAGNOSTICS
        ++g_search_diagnostics.aspiration_researches;
#endif
        window *= 2;
        if (score <= alpha)
          alpha = std::max(-INF_SCORE, previous_iteration_score - window);
        else
          beta = std::min(INF_SCORE, previous_iteration_score + window);
      }
    } else {
      score = alpha_beta_pruning(depth, board, tt, -INF_SCORE, INF_SCORE);
    }

    if (should_stop_search()) {
      g_search_stats.best_move.store(completed_move,
                                     std::memory_order_relaxed);
      g_search_stats.best_root_score.store(completed_score,
                                           std::memory_order_relaxed);
      break;
    }

    g_search_stats.highest_depth.store(depth, std::memory_order_relaxed);
    g_search_stats.best_root_score.store(score, std::memory_order_relaxed);
    previous_iteration_score = score;

    const Move current_move =
        g_search_stats.best_move.load(std::memory_order_relaxed);
    if (current_move == previous_iteration_move)
      ++stable_iterations;
    else
      stable_iterations = 0;
    previous_iteration_move = current_move;

    const int64_t soft_limit =
        g_search_state.soft_time_limit.load(std::memory_order_relaxed);
    if (soft_limit > 0 && stable_iterations >= 1 &&
        g_search_stats.elapsed_ms(g_search_state) >= soft_limit)
      break;
  }

  end_search();
}
