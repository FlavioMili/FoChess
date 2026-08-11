// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#include "uci.h"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>

#include "fen.h"
#include "helpers.h"
#include "move.h"
#include "search.h"

namespace UCI {

int64_t allocate_time(int64_t remaining_ms, int64_t increment_ms,
                      int moves_to_go) noexcept {
  if (remaining_ms <= 0) return 0;

  const int safe_moves_to_go = std::max(1, moves_to_go);
  const int64_t reserve =
      std::clamp<int64_t>(remaining_ms / 20, 1, 200);
  const int64_t maximum = std::max<int64_t>(1, remaining_ms - reserve);
  const int64_t target =
      remaining_ms / safe_moves_to_go + increment_ms / 2;
  return std::clamp<int64_t>(target, 1, maximum);
}

}  // namespace UCI

UCIengine::UCIengine() : board(FEN::parse()), tt() {}

UCIengine::~UCIengine() { stop(); }

void UCIengine::loop() {
  std::string line;
  while (getline(std::cin, line)) {
    if (line == "uci") {
      uci();
    } else if (line == "isready") {
      isready();
    } else if (line == "ucinewgame") {
      ucinewgame();
    } else if (line.rfind("position", 0) == 0) {
      position(line);
    } else if (line.rfind("go", 0) == 0) {
      go(line);
    } else if (line == "stop") {
      stop();
    } else if (line == "quit") {
      stop();  // Ensure search stops before quitting
      break;
    }
  }
  stop();
}

void UCIengine::uci() {
  std::cout << "id name FoChess\n";
  std::cout << "id author Flavio Milinanni\n";
  std::cout << "uciok" << std::endl;
}

void UCIengine::isready() {
  std::cout << "readyok" << std::endl;
}

void UCIengine::position(std::string& line) {
  stop();

  std::stringstream ss(line);
  std::string token;
  ss >> token;  // "position"
  ss >> token;  // "startpos" or "fen"

  if (token == "startpos") {
    board = FEN::parse();
  } else if (token == "fen") {
    std::string fenstr, tmp;
    while (ss >> tmp && tmp != "moves") {
      fenstr += tmp + " ";
    }
    board = FEN::parse(fenstr);
  }

  // Apply moves if present
  while (ss >> token) {
    if (token == "moves") continue;
    board.makeMove(PrintingHelpers::uci_to_move(token, board));
  }
}

void UCIengine::go(std::string& line) {
  // Stop any existing search
  stop();

  std::stringstream ss(line);
  std::string token;
  ss >> token;  // Skip "go"

  int depth = 64;  // Max depth by default
  int64_t movetime = 0;
  int64_t wtime = 0, btime = 0;
  int64_t winc = 0, binc = 0;
  int movestogo = 30;  // Default moves to go
  bool infinite = false;

  while (ss >> token) {
    if (token == "depth") {
      int temp_depth;
      ss >> temp_depth;
      depth = std::clamp(temp_depth, 1, 64);
    } else if (token == "movetime") {
      ss >> movetime;
    } else if (token == "wtime") {
      ss >> wtime;
    } else if (token == "btime") {
      ss >> btime;
    } else if (token == "winc") {
      ss >> winc;
    } else if (token == "binc") {
      ss >> binc;
    } else if (token == "movestogo") {
      ss >> movestogo;
    } else if (token == "infinite") {
      infinite = true;
    }
  }

  int64_t time_for_move = 0;

  if (movetime > 0) {
    const int64_t reserve =
        std::clamp<int64_t>(movetime / 20, 1, 50);
    time_for_move = std::max<int64_t>(1, movetime - reserve);
  } else if (!infinite) {
    int64_t our_time = (board.sideToMove == WHITE) ? wtime : btime;
    int64_t our_inc = (board.sideToMove == WHITE) ? winc : binc;

    if (our_time > 0) {
      time_for_move = UCI::allocate_time(our_time, our_inc, movestogo);
    }
  }

  FoChess::g_search_state.time_limit.store(time_for_move,
                                           std::memory_order_relaxed);
  FoChess::reset_search();
  is_searching = true;

  search_thread =
      std::thread(&UCIengine::search_thread_func, this, depth, time_for_move);
}

void UCIengine::search_thread_func(int depth,
                                  [[maybe_unused]] int64_t time_ms) {
  FoChess::iterative_deepening(depth, board, tt, false);

  Move best = FoChess::g_search_stats.best_move.load(std::memory_order_relaxed);

  info();
  if (best.raw() != EMPTY_MOVE) {
    std::string move_str = PrintingHelpers::move_to_str(best);
    std::cout << "bestmove " << move_str << std::endl;
  } else {
    std::cout << "bestmove 0000" << std::endl;
  }
  is_searching = false;
}

void UCIengine::info() const {
  int depth =
      FoChess::g_search_stats.highest_depth.load(std::memory_order_relaxed);

  int score =
      FoChess::g_search_stats.best_root_score.load(std::memory_order_relaxed);

  uint64_t nodes =
      FoChess::g_search_stats.node_count.load(std::memory_order_relaxed);

  int64_t time_ms = FoChess::g_search_stats.elapsed_ms(FoChess::g_search_state);

  std::cout << "info depth " << depth << " score cp " << score << " nodes "
            << nodes << " time " << time_ms << " nps "
            << static_cast<uint64_t>(
                   FoChess::g_search_stats.nps(FoChess::g_search_state))
            << std::endl;
}

void UCIengine::stop() {
  if (is_searching.load(std::memory_order_relaxed)) {
    FoChess::g_search_state.should_stop.store(true, std::memory_order_relaxed);
  }
  if (search_thread.joinable()) search_thread.join();
  is_searching.store(false, std::memory_order_relaxed);
}

void UCIengine::ucinewgame() {
  stop();  // Justin Case
  board = FEN::parse();
  tt.clear();
}
