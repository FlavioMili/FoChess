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

UCIengine::UCIengine() : board(FEN::parse()), tt() {}

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

  uint8_t depth = 64;  // Max depth by default
  int64_t movetime = 0;
  int64_t wtime = 0, btime = 0;
  int64_t winc = 0, binc = 0;
  int movestogo = 30;  // Default moves to go
  bool infinite = false;

  while (ss >> token) {
    if (token == "depth") {
      int temp_depth;
      ss >> temp_depth;
      depth = static_cast<uint8_t>(temp_depth);
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
    time_for_move = std::max<int64_t>(10, movetime - 50);
  } else if (!infinite) {
    int64_t our_time = (board.sideToMove == WHITE) ? wtime : btime;
    int64_t our_inc = (board.sideToMove == WHITE) ? winc : binc;

    if (our_time > 0) {
      time_for_move = our_time / 20 + our_inc / 2;
      time_for_move = std::clamp<int64_t>(time_for_move, 20, our_time - 200);
    }
  }

  FoChess::g_search_state.should_stop.store(false, std::memory_order_relaxed);
  FoChess::g_search_state.time_limit.store(time_for_move,
                                           std::memory_order_relaxed);
  is_searching = true;

  search_thread =
      std::thread(&UCIengine::search_thread_func, this, depth, time_for_move);
  search_thread.detach();
}

void UCIengine::search_thread_func(uint8_t depth, [[maybe_unused]] int64_t time_ms) {
  FoChess::iterative_deepening(depth, board, tt);

  Move best = FoChess::g_search_stats.best_move.load(std::memory_order_relaxed);

  if (best.raw() != EMPTY_MOVE) {
    info();
    std::string move_str = PrintingHelpers::move_to_str(best);
    std::cout << "bestmove " << move_str << std::endl;
  }
  is_searching = false;
}

void UCIengine::info() {
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
  if (is_searching) {
    FoChess::g_search_state.should_stop.store(true, std::memory_order_relaxed);

    // Wait for search to finish (with timeout)
    auto wait_start = std::chrono::steady_clock::now();
    while (is_searching) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));

      auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                         std::chrono::steady_clock::now() - wait_start)
                         .count();

      // Timeout after 100ms
      if (elapsed > 100) break;
    }
  }
}

void UCIengine::ucinewgame() {
  stop();  // Justin Case
  board = FEN::parse();
  tt.clear();
}
