// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#include "movegen.h"

#include <array>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <string>

#include "board.h"
#include "fen.h"
#include "helpers.h"

void TestBoardGeneration(std::string fen, size_t expected_moves) {
  Board board = FEN::parse(fen);

  std::cout << "Printing parsed board:\n";
  PrintingHelpers::printBoard(board);

  std::array<Move, MAX_MOVES> moves;

  size_t n_moves = MoveGen::generate_all(board, moves);

  std::cout << "\nGenerated " << n_moves << " moves:\n";
  assert(n_moves == expected_moves);
  for (size_t i = 0; i < n_moves; ++i) {
    std::cout << PrintingHelpers::nice_move_to_str(moves[i], board) << "\n";
  }
  std::cout << "En Passant: " <<
      PrintingHelpers::square_to_str(board.enPassant) << "\n\n\n";
}

int main() {
  std::string startFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -";
  TestBoardGeneration(startFEN, 20);

  std::string complexFEN = "8/8/8/1pP3n1/8/5k2/5N2/4K3 w - b6 0 2";
  TestBoardGeneration(complexFEN, 11);

  std::string onlyKingMovesFEN = "8/4k3/4q3/8/8/4N3/8/4K3 w - - 0 2";
  TestBoardGeneration(onlyKingMovesFEN, 5);

  std::string stalemateFEN ="8/8/8/8/8/1q6/8/K1k5 w - - 0 1";
  TestBoardGeneration(stalemateFEN, 0);

  return 0;
}
