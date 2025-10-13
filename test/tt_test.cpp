// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#include "tt.h"

#include <cassert>
#include <iostream>

#include "board.h"
#include "fen.h"
#include "helpers.h"
#include "search.h"
#include "zobrist.h"

int main() {
  Zobrist::init_zobrist_keys();

  // Initial position
  Board board1 = FEN::parse("8/8/8/8/8/8/8/k1K5 w - - 0 1");
  board1.pieces[WHITE][KNIGHT] |= (1ULL << Square::G1);
  board1.updateOccupancy();

  // Position after g1-f3-g1
  Board board2 = board1;
  board2.makeMove(PrintingHelpers::uci_to_move("g1f3"));
  board2.makeMove(PrintingHelpers::uci_to_move("a1a2"));  // dummy move
  board2.makeMove(PrintingHelpers::uci_to_move("f3g1"));
  board2.makeMove(PrintingHelpers::uci_to_move("a2a1"));  // dummy move

  std::cout << "Board 1 FEN: " << FEN::to_fen(board1) << std::endl;
  std::cout << "Board 2 FEN: " << FEN::to_fen(board2) << std::endl;

  assert(Zobrist::generate_hash(board1) == Zobrist::generate_hash(board2));

  TranspositionTable tt;

  SearchResult result1 = FoChess::negamax(5, board1, tt);
  SearchResult result2 = FoChess::negamax(5, board2, tt);

  assert(result1.score == result2.score);
  assert(result1.move == result2.move);

  std::cout << "Transposition table test passed!\n";

  return 0;
}
