// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#pragma once

#include <cstddef>
#include "board.h"
#include "types.h"

constexpr int MAX_MOVES = 218;

class MoveGen {
 public:
  MoveGen(const Board& b) : board(b), move_count(0) {}

  void generate_all();
  const std::array<Move, MAX_MOVES>& moves() const { return move_list; }
  size_t count() const { return move_count; }

 private:
  const Board& board;
  std::array<Move, MAX_MOVES> move_list;
  size_t move_count;

  void add_move(Square from, Square to, Piece promo = KNIGHT);

  void generate_pawn_moves(Color c, Square sq);
  void generate_knight_moves(Color c, Square sq);
  void generate_bishop_moves(Color c, Square sq);
  void generate_rook_moves(Color c, Square sq);
  void generate_queen_moves(Color c, Square sq);
  void generate_king_moves(Color c, Square sq);
};
