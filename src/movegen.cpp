// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#include "movegen.h"

#include <cassert>
#include <cstddef>

#include "bitboard.h"
#include "types.h"

namespace MoveGen {

size_t generate_all(const Board& board, std::array<Move, MAX_MOVES>& moves) {
  size_t n_moves = 0;
  const Color friendly_color = board.getSideToMove();
  const Color enemy_color = (friendly_color == WHITE) ? BLACK : WHITE;

  const Bitboard friendly = board.getOccupancy(friendly_color);
  const Bitboard enemy = board.getOccupancy(enemy_color);
  const Bitboard occ = friendly | enemy;
  const Bitboard empty = ~occ;

  // Helper lambda to test if a move is legal by making it on a temporary board
  auto is_legal = [&](const Move& m) -> bool {
    Board tmp = board;
    tmp.makeMove(m);
    return !tmp.is_in_check(friendly_color);
  };

  // Generate moves for each piece type
  for (size_t pt = PAWN; pt <= KING; ++pt) {
    Bitboard bb = board.getPieceBitboard(friendly_color, static_cast<Piece>(pt));

    while (bb) {
      Square from = static_cast<Square>(__builtin_ctzll(bb));
      Bitboard targets = 0;

      switch (pt) {
        case PAWN: {
          // --- Pawn pushes ---
          const uint8_t promotion_rank = (friendly_color == WHITE) ? 7 : 0;
          Bitboard pushes = Bitboards::pawn_moves(from, friendly_color, empty);
          while (pushes) {
            Square to = static_cast<Square>(__builtin_ctzll(pushes));
            if (Bitboards::rank_of(to) == promotion_rank) {
              if (is_legal(Move(from, to, QUEEN))) moves[n_moves++] = Move(from, to, QUEEN);
              if (is_legal(Move(from, to, ROOK))) moves[n_moves++] = Move(from, to, ROOK);
              if (is_legal(Move(from, to, BISHOP))) moves[n_moves++] = Move(from, to, BISHOP);
              if (is_legal(Move(from, to, KNIGHT))) moves[n_moves++] = Move(from, to, KNIGHT);
            } else {
              Move m = Move(from, to);
              if (is_legal(m)) moves[n_moves++] = m;
            }
            pushes &= pushes - 1;
          }

          // --- Pawn captures ---
          Bitboard attacks = Bitboards::pawn_attacks(from, friendly_color, enemy);
          while (attacks) {
            Square to = static_cast<Square>(__builtin_ctzll(attacks));
            if (Bitboards::rank_of(to) == promotion_rank) {
              if (is_legal(Move(from, to, QUEEN))) moves[n_moves++] = Move(from, to, QUEEN);
              if (is_legal(Move(from, to, ROOK))) moves[n_moves++] = Move(from, to, ROOK);
              if (is_legal(Move(from, to, BISHOP))) moves[n_moves++] = Move(from, to, BISHOP);
              if (is_legal(Move(from, to, KNIGHT))) moves[n_moves++] = Move(from, to, KNIGHT);
            } else {
              Move m = Move(from, to);
              if (is_legal(m)) moves[n_moves++] = m;
            }
            attacks &= attacks - 1;
          }

          // --- En passant ---
          if (board.getEnPassant() != NO_SQUARE) {
            if (Bitboards::pawn_attacks_mask(from, friendly_color) &
                Bitboards::square_bb(board.getEnPassant())) {
              Move m = Move(from, board.getEnPassant(), EN_PASSANT);
              if (is_legal(m)) {
                moves[n_moves++] = m;
              }
            }
          }
          break;
        }
        case KNIGHT: {
          targets = Bitboards::knight_moves(from, friendly);
          while (targets) {
            Square to = static_cast<Square>(__builtin_ctzll(targets));
            Move m = Move(from, to);
            if (is_legal(m)) moves[n_moves++] = m;
            targets &= targets - 1;
          }
          break;
        }
        case BISHOP: {
          targets = Bitboards::bishop_moves(from, occ, friendly);
          while (targets) {
            Square to = static_cast<Square>(__builtin_ctzll(targets));
            Move m = Move(from, to);
            if (is_legal(m)) moves[n_moves++] = m;
            targets &= targets - 1;
          }
          break;
        }
        case ROOK: {
          targets = Bitboards::rook_moves(from, occ, friendly);
          while (targets) {
            Square to = static_cast<Square>(__builtin_ctzll(targets));
            Move m = Move(from, to);
            if (is_legal(m)) moves[n_moves++] = m;
            targets &= targets - 1;
          }
          break;
        }
        case QUEEN: {
          targets = Bitboards::queen_moves(from, occ, friendly);
          while (targets) {
            Square to = static_cast<Square>(__builtin_ctzll(targets));
            Move m = Move(from, to);
            if (is_legal(m)) moves[n_moves++] = m;
            targets &= targets - 1;
          }
          break;
        }
        case KING: {
          targets = Bitboards::king_moves(from, friendly);
          while (targets) {
            Square to = static_cast<Square>(__builtin_ctzll(targets));
            Move m = Move(from, to);
            if (is_legal(m)) moves[n_moves++] = m;
            targets &= targets - 1;
          }

          // Castling
          CastlingRights cr = board.getCastlingRights();
          if (friendly_color == WHITE) {
            if (cr.whiteKingside && (occ & Bitboards::WK_EMPTY) == 0 && !board.is_in_check(WHITE)) {
              if (!board.attacks_to(F1, enemy_color) && !board.attacks_to(G1, enemy_color)) {
                moves[n_moves++] = Move(E1, G1, CASTLING);
              }
            }
            if (cr.whiteQueenside && (occ & Bitboards::WQ_EMPTY) == 0 &&
                !board.is_in_check(WHITE)) {
              if (!board.attacks_to(D1, enemy_color) && !board.attacks_to(C1, enemy_color)) {
                moves[n_moves++] = Move(E1, C1, CASTLING);
              }
            }
          } else {  // BLACK
            if (cr.blackKingside && (occ & Bitboards::BK_EMPTY) == 0 && !board.is_in_check(BLACK)) {
              if (!board.attacks_to(F8, enemy_color) && !board.attacks_to(G8, enemy_color)) {
                moves[n_moves++] = Move(E8, G8, CASTLING);
              }
            }
            if (cr.blackQueenside && (occ & Bitboards::BQ_EMPTY) == 0 &&
                !board.is_in_check(BLACK)) {
              if (!board.attacks_to(D8, enemy_color) && !board.attacks_to(C8, enemy_color)) {
                moves[n_moves++] = Move(E8, C8, CASTLING);
              }
            }
          }
          break;
        }
      }
      bb &= bb - 1;
    }
  }
  return n_moves;
}

}  // namespace MoveGen
