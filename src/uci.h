// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#pragma once

#include <string>
#include <thread>

#include "board.h"
#include "tt.h"

class UCIengine {
 public:
  UCIengine();
  void loop();

 private:
  void uci();
  void isready();
  void position(std::string& line);
  void go(std::string& line);
  void stop();
  void ucinewgame();
  void info(int depth, int score);

  void search_thread_func(uint8_t depth, int64_t time_ms);

  Board board;
  TranspositionTable tt;

  std::thread search_thread;
  std::atomic<bool> is_searching{false};
};
