// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#include "movegen.h"

#include <cassert>
#include <cstddef>
#include <vector>

#include "bitboard.h"
#include "types.h"

namespace MoveGen {

std::vector<Move> generate_all(const Board& board) {
  std::vector<Move> move_list;
  move_list.reserve(MAX_MOVES);

  const Color side = board.sideToMove;
  const Color enemy = (side == WHITE ? BLACK : WHITE);

  const Bitboard occ = board.occupancy[WHITE] | board.occupancy[BLACK];
  const Bitboard friendly = board.occupancy[side];
  const Bitboard enemies = board.occupancy[enemy];
  const Bitboard empty = ~occ;

  for (size_t piece = PAWN; piece <= KING; ++piece) {
    Bitboard bb = board.pieces[side][piece];
    while (bb) {
      Square from = static_cast<Square>(__builtin_ctzll(bb));
      Bitboard targets = 0;

      switch (piece) {
        case PAWN:
          targets = Bitboards::pawn_moves(from, side, empty, enemies);
          break;
        case KNIGHT:
          targets = Bitboards::knight_moves(from, friendly);
          break;
        case BISHOP:
          targets = Bitboards::bishop_moves(from, occ, friendly);
          break;
        case ROOK:
          targets = Bitboards::rook_moves(from, occ, friendly);
          break;
        case QUEEN:
          targets = Bitboards::queen_moves(from, occ, friendly);
          break;
        case KING:
          targets = Bitboards::king_moves(from, friendly);
          break;
      }

      while (targets) {
        Square to = static_cast<Square>(__builtin_ctzll(targets));
        move_list.push_back(Move(from, to));
        targets &= targets - 1;
      }

      bb &= bb - 1;
    }
  }

  return move_list;
}

void generate_pawn_moves(const Board& board, Color c, std::vector<Move>& moves) {
  Bitboard occ = board.occupancy[WHITE] | board.occupancy[BLACK];
  Bitboard friendly = board.occupancy[c];
  Bitboard pawns = board.pieces[c][PAWN];

  while (pawns) {
    Square from = static_cast<Square>(__builtin_ctzll(pawns));
    Bitboard targets = Bitboards::pawn_moves(from, c, ~occ, friendly);

    while (targets) {
      Square to = static_cast<Square>(__builtin_ctzll(targets));
      moves.push_back(Move(from, to, KNIGHT, MoveType::NORMAL));
      targets &= targets - 1;
    }

    pawns &= pawns - 1;
  }
}

void generate_knight_moves(const Board& board, Color c, std::vector<Move>& moves) {
  Bitboard friendly = board.occupancy[c];
  Bitboard knights = board.pieces[c][KNIGHT];

  while (knights) {
    Square from = static_cast<Square>(__builtin_ctzll(knights));
    Bitboard targets = Bitboards::knight_moves(from, friendly);

    while (targets) {
      Square to = static_cast<Square>(__builtin_ctzll(targets));
      moves.push_back(Move(from, to, KNIGHT, MoveType::NORMAL));
      targets &= targets - 1;
    }

    knights &= knights - 1;
  }
}

void generate_bishop_moves(const Board& board, Color c, std::vector<Move>& moves) {
  Bitboard occ = board.occupancy[WHITE] | board.occupancy[BLACK];
  Bitboard friendly = board.occupancy[c];
  Bitboard bishops = board.pieces[c][BISHOP];

  while (bishops) {
    Square from = static_cast<Square>(__builtin_ctzll(bishops));
    Bitboard targets = Bitboards::bishop_moves(from, occ, friendly);

    while (targets) {
      Square to = static_cast<Square>(__builtin_ctzll(targets));
      moves.push_back(Move(from, to, KNIGHT, MoveType::NORMAL));
      targets &= targets - 1;
    }

    bishops &= bishops - 1;
  }
}

void generate_rook_moves(const Board& board, Color c, std::vector<Move>& moves) {
  Bitboard occ = board.occupancy[WHITE] | board.occupancy[BLACK];
  Bitboard friendly = board.occupancy[c];
  Bitboard rooks = board.pieces[c][ROOK];

  while (rooks) {
    Square from = static_cast<Square>(__builtin_ctzll(rooks));
    Bitboard targets = Bitboards::rook_moves(from, occ, friendly);

    while (targets) {
      Square to = static_cast<Square>(__builtin_ctzll(targets));
      moves.push_back(Move(from, to, KNIGHT, MoveType::NORMAL));
      targets &= targets - 1;
    }

    rooks &= rooks - 1;
  }
}

void generate_queen_moves(const Board& board, Color c, std::vector<Move>& moves) {
  Bitboard occ = board.occupancy[WHITE] | board.occupancy[BLACK];
  Bitboard friendly = board.occupancy[c];
  Bitboard queens = board.pieces[c][QUEEN];

  while (queens) {
    Square from = static_cast<Square>(__builtin_ctzll(queens));
    Bitboard targets = Bitboards::queen_moves(from, occ, friendly);

    while (targets) {
      Square to = static_cast<Square>(__builtin_ctzll(targets));
      moves.push_back(Move(from, to, KNIGHT, MoveType::NORMAL));
      targets &= targets - 1;
    }

    queens &= queens - 1;
  }
}

void generate_king_moves(const Board& board, Color c, std::vector<Move>& moves) {
  Bitboard friendly = board.occupancy[c];
  Bitboard kings = board.pieces[c][KING];

  while (kings) {
    Square from = static_cast<Square>(__builtin_ctzll(kings));
    Bitboard targets = Bitboards::king_moves(from, friendly);

    while (targets) {
      Square to = static_cast<Square>(__builtin_ctzll(targets));
      moves.push_back(Move(from, to, KNIGHT, MoveType::NORMAL));
      targets &= targets - 1;
    }

    kings &= kings - 1;
  }
}

}  // namespace MoveGen
