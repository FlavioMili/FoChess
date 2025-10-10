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

namespace UCI {

void loop() {
  std::string line;
  while (getline(std::cin, line)) {
    if (line == "uci") {
      std::cout << "id name FoChess\n";
      std::cout << "id author Flavio Milinanni\n";
      std::cout << "uciok\n";
    } else if (line == "isready") {
      std::cout << "readyok\n";
    } else if (line == "ucinewgame") {
      // reset board / hash
    } else if (line.rfind("position", 0) == 0) {
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
    } else if (line.rfind("go", 0) == 0) {
      // run search, print best move
      std::string move = PrintingHelpers::move_to_str(FoChess::negamax(5, testboard).move);
      std::cout << "bestmove " << move << "\n";
    } else if (line == "stop") {
      // handle stop
    } else if (line == "quit") {
      break;
    }
  }
}

/**********************************************************************************
********************************** GUI TO ENGINE **********************************
**********************************************************************************/

/**********************************************************************************
********************************** ENGINE TO GUI **********************************
**********************************************************************************/
}  // namespace UCI
