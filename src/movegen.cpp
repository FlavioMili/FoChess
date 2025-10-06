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

size_t generate_all(const Board& board, std::array<Move, MAX_MOVES>& moves) {
  size_t n_moves = 0;
  const Color us = board.sideToMove;
  const Color them = (us == WHITE) ? BLACK : WHITE;
  
  const Bitboard occ = board.occupancy[WHITE] | board.occupancy[BLACK];
  const Bitboard friendly = board.occupancy[us];
  const Bitboard enemies = board.occupancy[them];
  const Bitboard empty = ~occ;

  // Helper lambda to test if a move is legal
  auto is_legal = [&](Square from, Square to) -> bool {
    Board tmp = board;
    tmp.makeMove(Move(from, to));
    return !tmp.is_in_check(us);
  };

  // Helper lambda to add moves from a target bitboard
  auto add_moves = [&](Square from, Bitboard targets, bool check_legality) {
    while (targets) {
      Square to = static_cast<Square>(__builtin_ctzll(targets));
      if (!check_legality || is_legal(from, to)) {
        moves[n_moves++] = Move(from, to);
      }
      targets &= targets - 1;
    }
  };

  // Compute enemy attack map (for king moves)
  Bitboard enemy_attacks = 0;
  for (size_t pt = PAWN; pt <= KING; ++pt) {
    Bitboard bb = board.pieces[them][pt];
    while (bb) {
      Square sq = static_cast<Square>(__builtin_ctzll(bb));
      switch (pt) {
        case PAWN:   enemy_attacks |= Bitboards::pawn_attacks_mask(sq, them); break;
        case KNIGHT: enemy_attacks |= Bitboards::knight_attacks(sq); break;
        case BISHOP: enemy_attacks |= Bitboards::bishop_attacks(sq, occ); break;
        case ROOK:   enemy_attacks |= Bitboards::rook_attacks(sq, occ); break;
        case QUEEN:  enemy_attacks |= Bitboards::queen_attacks(sq, occ); break;
        case KING:   enemy_attacks |= Bitboards::king_attacks(sq); break;
      }
      bb &= bb - 1;
    }
  }

  // Generate moves for each piece type
  for (size_t pt = PAWN; pt <= KING; ++pt) {
    Bitboard bb = board.pieces[us][pt];
    
    while (bb) {
      Square from = static_cast<Square>(__builtin_ctzll(bb));
      Bitboard targets = 0;

      switch (pt) {
        case PAWN: {
          targets = Bitboards::pawn_moves(from, us, empty, enemies);
          
          if (board.enPassant != NO_SQUARE) {
            Bitboard ep_attacks = Bitboards::pawn_attacks_mask(from, us);
            if (ep_attacks & Bitboards::square_bb(board.enPassant)) {
              targets |= Bitboards::square_bb(board.enPassant);
            }
          }
          add_moves(from, targets, true);
          break;
        }
        case KNIGHT:
          targets = Bitboards::knight_moves(from, friendly);
          add_moves(from, targets, true);
          break;
        case BISHOP:
          targets = Bitboards::bishop_moves(from, occ, friendly);
          add_moves(from, targets, true);
          break;
        case ROOK:
          targets = Bitboards::rook_moves(from, occ, friendly);
          add_moves(from, targets, true);
          break;
        case QUEEN:
          targets = Bitboards::queen_moves(from, occ, friendly);
          add_moves(from, targets, true);
          break;
        case KING:
          // King can't move into check
          targets = Bitboards::king_moves(from, friendly) & ~enemy_attacks;
          add_moves(from, targets, false);
          break;
      }
      
      bb &= bb - 1;
    }
  }

  return n_moves;
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
