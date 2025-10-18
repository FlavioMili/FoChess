// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#include "movegen.h"

#include <array>
#include <cstddef>

#include "bitboard.h"
#include "magic.h"
#include "types.h"

namespace MoveGen {

alignas(64) std::array<Move, MAX_MOVES> candidate_moves;
size_t generate_all(const Board& board, std::array<Move, MAX_MOVES>& moves) {
  auto n_candidate_moves = generate_pseudolegal(board, candidate_moves);
  size_t idx = 0;
  // Board tmp = board;
  // auto friendly_color = board.sideToMove;
  for (size_t i = 0; i < n_candidate_moves; ++i) {
    // tmp = board;
    // tmp.makeMove(candidate_moves[i]);
    // if (!tmp.is_in_check(friendly_color)) moves[idx++] = candidate_moves[i];
    if (board.isLegalMove(candidate_moves[i])) moves[idx++] = candidate_moves[i];
  }
  return idx;
}

size_t generate_pseudolegal(const Board& board, std::array<Move, MAX_MOVES>& moves) {
  size_t n_candidate_moves = 0;

  const Color friendly_color = board.sideToMove;
  const Color enemy_color = (friendly_color == WHITE) ? BLACK : WHITE;

  const Bitboard friendly = board.occupancy[friendly_color];
  const Bitboard enemy = board.occupancy[enemy_color];
  const Bitboard empty = ~board.allPieces;

  auto add_promotions = [&](Square from, Square to) {
    moves[n_candidate_moves++] = Move(from, to, QUEEN);
    moves[n_candidate_moves++] = Move(from, to, ROOK);
    moves[n_candidate_moves++] = Move(from, to, BISHOP);
    moves[n_candidate_moves++] = Move(from, to, KNIGHT);
  };

  // Generate moves for each piece type
  for (size_t pt = PAWN; pt <= KING; ++pt) {
    Bitboard bb = board.pieces[friendly_color][static_cast<Piece>(pt)];

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
            if (Bitboards::rank_of(to) == promotion_rank) [[unlikely]] {
              add_promotions(from, to);
            } else {
              moves[n_candidate_moves++] = Move(from, to);
            }
            pushes &= pushes - 1;
          }

          // --- Pawn captures ---
          Bitboard attacks = Bitboards::pawn_attacks(from, friendly_color, enemy);
          while (attacks) {
            Square to = static_cast<Square>(__builtin_ctzll(attacks));
            if (Bitboards::rank_of(to) == promotion_rank) [[unlikely]] {
              add_promotions(from, to);
            } else {
              moves[n_candidate_moves++] = Move(from, to);
            }
            attacks &= attacks - 1;
          }

          Square enp = board.enPassant;
          // --- En passant ---
          if (enp != NO_SQUARE &&
              Bitboards::pawn_attacks_mask(from, friendly_color) & Bitboards::square_bb(enp)) {
            moves[n_candidate_moves++] = Move(from, enp, EN_PASSANT);
          }
          break;
        }
        case KNIGHT: {
          targets = Bitboards::knight_moves(from, friendly);
          while (targets) {
            Square to = static_cast<Square>(__builtin_ctzll(targets));
            moves[n_candidate_moves++] = Move(from, to);
            targets &= targets - 1;
          }
          break;
        }
        case BISHOP: {
          targets = Bitboards::bishop_moves(from, board.allPieces, friendly);
          while (targets) {
            Square to = static_cast<Square>(__builtin_ctzll(targets));
            moves[n_candidate_moves++] = Move(from, to);
            ;
            targets &= targets - 1;
          }
          break;
        }
        case ROOK: {
          targets = Bitboards::rook_moves(from, board.allPieces, friendly);
          while (targets) {
            Square to = static_cast<Square>(__builtin_ctzll(targets));
            moves[n_candidate_moves++] = Move(from, to);
            targets &= targets - 1;
          }
          break;
        }
        case QUEEN: {
          targets = Bitboards::queen_moves(from, board.allPieces, friendly);
          while (targets) {
            Square to = static_cast<Square>(__builtin_ctzll(targets));
            moves[n_candidate_moves++] = Move(from, to);
            targets &= targets - 1;
          }
          break;
        }
        case KING: {
          targets = Bitboards::king_moves(from, friendly);
          while (targets) {
            Square to = static_cast<Square>(__builtin_ctzll(targets));
            moves[n_candidate_moves++] = Move(from, to);
            targets &= targets - 1;
          }

          // Castling
          CastlingRights cr = board.castling;
          if (friendly_color == WHITE) {
            if (cr.whiteKingside && (board.allPieces & Bitboards::WK_EMPTY) == 0 &&
                !board.is_in_check(WHITE)) {
              if (!board.attacks_to(F1, enemy_color) && !board.attacks_to(G1, enemy_color)) {
                moves[n_candidate_moves++] = Move(E1, G1, CASTLING);
              }
            }
            if (cr.whiteQueenside && (board.allPieces & Bitboards::WQ_EMPTY) == 0 &&
                !board.is_in_check(WHITE)) {
              if (!board.attacks_to(D1, enemy_color) && !board.attacks_to(C1, enemy_color)) {
                moves[n_candidate_moves++] = Move(E1, C1, CASTLING);
              }
            }
          } else {  // BLACK
            if (cr.blackKingside && (board.allPieces & Bitboards::BK_EMPTY) == 0 &&
                !board.is_in_check(BLACK)) {
              if (!board.attacks_to(F8, enemy_color) && !board.attacks_to(G8, enemy_color)) {
                moves[n_candidate_moves++] = Move(E8, G8, CASTLING);
              }
            }
            if (cr.blackQueenside && (board.allPieces & Bitboards::BQ_EMPTY) == 0 &&
                !board.is_in_check(BLACK)) {
              if (!board.attacks_to(D8, enemy_color) && !board.attacks_to(C8, enemy_color)) {
                moves[n_candidate_moves++] = Move(E8, C8, CASTLING);
              }
            }
          }
          break;
        }
        default:
          break;
      }
      bb &= bb - 1;
    }
  }

  return n_candidate_moves;
}

}  // namespace MoveGen
