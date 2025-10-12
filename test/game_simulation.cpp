// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#include <array>
#include <cstddef>
#include <iostream>

#include "board.h"
#include "fen.h"
#include "helpers.h"
#include "move.h"
#include "movegen.h"
#include "search.h"
#include "zobrist.h"

int main() {
  Board board = FEN::parse("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -");
  Zobrist::init_zobrist_keys();
  TranspositionTable tt;


  while (true) {
    std::array<Move, MAX_MOVES> moves;
    size_t n = MoveGen::generate_all(board, moves);
    if (n == 0) break;


    SearchResult result = FoChess::alpha_beta_pruning(6, board, tt);
    board.makeMove(result.move);
    std::cout << " best move score: " << result.score << "\n";

    PrintingHelpers::printBoard(board);
  }

  return 0;
}
