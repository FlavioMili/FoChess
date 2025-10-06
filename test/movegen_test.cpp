// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#include "movegen.h"

#include <array>
#include <cstddef>
#include <iostream>
#include <string>

#include "board.h"
#include "fen.h"
#include "helpers.h"

void TestBoardGeneration(std::string fen) {
  Board board = FEN::parse(fen);

  std::cout << "Printing parsed board:\n";
  PrintingHelpers::printBoard(board);

  std::array<Move, MAX_MOVES> moves;

  size_t n_moves = MoveGen::generate_all(board, moves);

  std::cout << "\nGenerated " << n_moves << " moves:\n";
  for (size_t i = 0; i < n_moves; ++i) {
    std::cout << PrintingHelpers::move_to_str(moves[i], board) << "\n";
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
