// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2026 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#pragma once

#include <array>
#include <cstdint>

#include "board.h"
#include "movegen.h"

// Bulk counting is the default. Debug tools can visit every leaf to collect
// statistics without duplicating move generation or board updates.
template <bool Bulk = true, class LeafVisitor>
uint64_t perft(const Board& board, int depth, const LeafVisitor& visit) {
  if (depth <= 0) return 1;

  std::array<Move, MAX_MOVES> moves;
  const size_t n = MoveGen::generate_all(board, moves);
  if constexpr (Bulk) {
    if (depth == 1) return n;
  }

  uint64_t nodes = 0;
  for (size_t i = 0; i < n; ++i) {
    Board child = board;
    child.makeMove(moves[i]);
    if (depth == 1) visit(child);
    nodes += perft<Bulk>(child, depth - 1, visit);
  }
  return nodes;
}

template <bool Bulk = true>
uint64_t perft(const Board& board, int depth) {
  return perft<Bulk>(board, depth, [](const Board&) {});
}
