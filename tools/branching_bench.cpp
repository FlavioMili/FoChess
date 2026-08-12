#include <array>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

#include "fen.h"
#include "magic.h"
#include "search.h"
#include "tt.h"
#include "zobrist.h"

#ifndef SEARCH_DIAGNOSTICS
#error "Compile branching_bench with -DSEARCH_DIAGNOSTICS"
#endif

namespace {

void add(FoChess::SearchDiagnostics& total, const FoChess::SearchDiagnostics& sample) {
  total.main_nodes += sample.main_nodes;
  total.quiescence_nodes += sample.quiescence_nodes;
  total.main_generated_moves += sample.main_generated_moves;
  total.main_searched_moves += sample.main_searched_moves;
  total.quiescence_generated_moves += sample.quiescence_generated_moves;
  total.quiescence_searched_moves += sample.quiescence_searched_moves;
  total.quiescence_recursive_moves += sample.quiescence_recursive_moves;
  total.stand_pat_cutoffs += sample.stand_pat_cutoffs;
  total.delta_prunes += sample.delta_prunes;
  total.quiescence_beta_cutoffs += sample.quiescence_beta_cutoffs;
  total.quiescence_repeats += sample.quiescence_repeats;
  total.quiescence_tt_cutoffs += sample.quiescence_tt_cutoffs;
  total.beta_cutoffs += sample.beta_cutoffs;
  for (size_t i = 0; i < total.cutoff_buckets.size(); ++i)
    total.cutoff_buckets[i] += sample.cutoff_buckets[i];
  total.lmr_searches += sample.lmr_searches;
  total.lmr_researches += sample.lmr_researches;
  total.null_searches += sample.null_searches;
  total.null_cutoffs += sample.null_cutoffs;
  total.tt_cutoffs += sample.tt_cutoffs;
  total.aspiration_searches += sample.aspiration_searches;
  total.aspiration_researches += sample.aspiration_researches;
}

double percent(uint64_t part, uint64_t whole) {
  return whole == 0 ? 0.0 : 100.0 * static_cast<double>(part) / static_cast<double>(whole);
}

}  // namespace

int main(int argc, char** argv) {
  const int max_depth = argc > 1 ? std::atoi(argv[1]) : 8;
  std::ifstream input("resources/bench_fen_list.txt");
  if (!input) return 1;

  std::array<std::string, 6> positions;
  size_t count = 0;
  for (std::string line; count < positions.size() && std::getline(input, line);)
    if (!line.empty() && line[0] != '#') positions[count++] = line;

  Zobrist::init_zobrist_keys();
  Bitboards::init_magic_tables();
  uint64_t previous_nodes = 0;

  std::cout << "depth nodes ebf qshare main_gen main_search q_gen q_recurse "
               "stand_cut delta_cut qbeta_cut qrepeat qtt_cut first_cut lmr_retry null_cut "
               "tt_cut asp_retry cutoffs[1,2,3,4,5-8,9+]\n";
  for (int depth = 1; depth <= max_depth; ++depth) {
    FoChess::SearchDiagnostics total;
    uint64_t nodes = 0;
    for (size_t i = 0; i < count; ++i) {
      Board board = FEN::parse(positions[i]);
      TranspositionTable tt;
      FoChess::reset_search_diagnostics();
      FoChess::iterative_deepening(depth, board, tt);
      nodes += FoChess::g_search_stats.node_count.load();
      add(total, FoChess::g_search_diagnostics);
    }

    const double ebf = previous_nodes == 0 ? 0.0 : static_cast<double>(nodes) / previous_nodes;
    const uint64_t all_nodes = total.main_nodes + total.quiescence_nodes;
    std::cout << depth << ' ' << nodes << ' ' << std::fixed << std::setprecision(2) << ebf << ' '
              << percent(total.quiescence_nodes, all_nodes) << ' '
              << (total.main_nodes == 0
                      ? 0.0
                      : static_cast<double>(total.main_generated_moves) / total.main_nodes)
              << ' '
              << (total.main_nodes == 0
                      ? 0.0
                      : static_cast<double>(total.main_searched_moves) / total.main_nodes)
              << ' '
              << (total.quiescence_nodes == 0
                      ? 0.0
                      : static_cast<double>(total.quiescence_generated_moves) /
                            total.quiescence_nodes)
              << ' '
              << (total.quiescence_nodes == 0
                      ? 0.0
                      : static_cast<double>(total.quiescence_recursive_moves) /
                            total.quiescence_nodes)
              << ' ' << percent(total.stand_pat_cutoffs, total.quiescence_nodes) << ' '
              << percent(total.delta_prunes, total.quiescence_searched_moves) << ' '
              << percent(total.quiescence_beta_cutoffs, total.quiescence_nodes) << ' '
              << percent(total.quiescence_repeats, total.quiescence_nodes) << ' '
              << percent(total.quiescence_tt_cutoffs, total.quiescence_nodes) << ' '
              << percent(total.cutoff_buckets[0], total.beta_cutoffs) << ' '
              << percent(total.lmr_researches, total.lmr_searches) << ' '
              << percent(total.null_cutoffs, total.null_searches) << ' '
              << percent(total.tt_cutoffs, all_nodes) << ' '
              << percent(total.aspiration_researches, total.aspiration_searches) << " [";
    for (size_t i = 0; i < total.cutoff_buckets.size(); ++i) {
      if (i != 0) std::cout << ',';
      std::cout << total.cutoff_buckets[i];
    }
    std::cout << "]\n";
    previous_nodes = nodes;
  }
}
