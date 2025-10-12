// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#include "uci.h"

#include <iostream>
#include <sstream>
#include <string>

#include "board.h"
#include "fen.h"
#include "helpers.h"
#include "search.h"

static Board testboard = FEN::parse();
static TranspositionTable tt; 

namespace UCI {

void loop() {
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
      go();
    } else if (line == "stop") {
      stop();
    } else if (line == "quit") {
      break;
    }
  }
}

/**********************************************************************************
********************************** GUI TO ENGINE **********************************
**********************************************************************************/

void uci() {
  std::cout << "id name FoChess\n id author Flavio Milinanni\n uciok\n";
}

void isready() {
  std::cout << "readyok\n";
}

void position(std::string line) {
  std::stringstream ss(line);
  std::string token;
  ss >> token;  // "position"
  ss >> token;  // "startpos" or "fen"

  if (token == "startpos") {
    testboard = FEN::parse();
  } else if (token == "fen") {
    std::string fenstr, tmp;
    while (ss >> tmp && tmp != "moves") {
      fenstr += tmp + " ";
    }
    testboard = FEN::parse(fenstr);
  }

  // apply moves if present
  while (ss >> token) {
    if (token == "moves") continue;
    testboard.makeMove(PrintingHelpers::uci_to_move(token));
  }
}

void go() {
  std::string move = PrintingHelpers::move_to_str(FoChess::negamax(7, testboard, tt).move);
  std::cout << "bestmove " << move << "\n";
}

void stop() {
  // i do not know yet 
}

void ucinewgame() {

}

/**********************************************************************************
********************************** ENGINE TO GUI **********************************
**********************************************************************************/
}  // namespace UCI
