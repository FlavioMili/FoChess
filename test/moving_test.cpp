// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#include <iostream>
#include <string>
#include "board.h"
#include "fen.h"
#include "helpers.h"
#include "types.h"
#include "zobrist.h"

int main () {
  std::string startFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -";
  Board board = FEN::parse(startFEN);

  PrintingHelpers::printBoard(board);
  std::cout << Zobrist::generate_hash(board) << "\n";

  board.makeMove(Move(Square::E2, Square::E3));
  PrintingHelpers::printBoard(board);
  std::cout << Zobrist::generate_hash(board) << "\n";

  board.makeMove(Move(Square::E3, Square::E8));
  PrintingHelpers::printBoard(board);
  std::cout << Zobrist::generate_hash(board) << "\n";

  return 0;
}
