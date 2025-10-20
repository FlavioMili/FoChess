// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#include "uci.h"

#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>

#include "fen.h"
#include "helpers.h"
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
      break;
    }
  }
}

void UCIengine::uci() {
  std::cout << "id name FoChess\n id author Flavio Milinanni\n uciok\n";
}

void UCIengine::isready() {
  std::cout << "readyok\n";
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

  // apply moves if present
  while (ss >> token) {
    if (token == "moves") continue;
    board.makeMove(PrintingHelpers::uci_to_move(token));
  }
}

void UCIengine::go(std::string& line) {
  std::stringstream ss(line);
  std::string token;
  ss >> token;  // Skip "go"

  uint8_t depth = 5;

  while (ss >> token) {
    if (token == "depth") {
      int temp_depth;
      ss >> temp_depth;
      depth = static_cast<uint8_t>(temp_depth);
    }
    // TODO: Add other UCI parameters like movetime, wtime, btime, etc.
  }

  auto result = FoChess::alpha_beta_pruning(depth, board, tt);
  info(depth, result.score);
  std::string move_str = PrintingHelpers::move_to_str(result.move);
  std::cout << "bestmove " << move_str << std::endl;
}

void UCIengine::info(int depth, int score) {
  double eval = score / 100.0;
  std::cout << "info depth " << static_cast<int>(depth)
            << " score cp " << score
            << " curr_eval " << eval
            << " nodes " << FoChess::nodes
            << std::endl;
  FoChess::nodes = 0;
}

void UCIengine::stop() {
  // TODO: Implement stop functionality
}

void UCIengine::ucinewgame() {
  board = FEN::parse();
  tt.clear();
  // TODO: Implement any other necessary new game logic
}
