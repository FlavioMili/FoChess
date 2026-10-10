// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2026 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <string>
#include <thread>
#include <vector>

#include "board.h"
#include "tt.h"

namespace UCI {
int64_t allocate_time(int64_t remaining_ms, int64_t increment_ms,
                      int moves_to_go) noexcept;
int64_t allocate_hard_time(int64_t remaining_ms,
                           int64_t soft_time_ms) noexcept;
}

class UCIengine {
 public:
  UCIengine();
  ~UCIengine();
  void loop();

 private:
  void uci();
  void isready();
  void position(std::string& line);
  void go(std::string& line);
  void stop();
  void ucinewgame();
  void info() const;

  void search_thread_func(int depth, int64_t time_ms);

  Board board;
  std::vector<uint64_t> position_history;
  TranspositionTable tt;

  std::thread search_thread;
  std::atomic<bool> is_searching{false};
};
