// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------


#include "movegen.h"

#include <iostream>
#include <string>
#include <vector>

#include "board.h"
#include "helpers.h"
#include "fen.h"


void TestBoardGeneration(std::string fen) {
  Board board = FEN::parse(fen);

  std::cout << "Printing parsed board:\n";
  PrintingHelpers::printBoard(board);

  std::vector<Move> mg = MoveGen::generate_all(board);

  std::cout << "\nGenerated " << mg.size() << " moves:\n";
  for (const auto& move : mg) {
    std::cout << PrintingHelpers::move_to_str(move, board) << "\n";
  }
  std::cout << "\n\n\n";
}

int main() {
  std::string startFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -";
  TestBoardGeneration(startFEN);

  std::string complexFEN = "8/8/8/1pP3n1/8/5k2/5N2/4K3 w - b6 0 2";
  TestBoardGeneration(complexFEN);

  return 0;
}
