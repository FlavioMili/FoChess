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

namespace {

Move* add_promotions(Square from, Square to, Move* move_list) {
  *move_list++ = Move(from, to, QUEEN);
  *move_list++ = Move(from, to, ROOK);
  *move_list++ = Move(from, to, BISHOP);
  *move_list++ = Move(from, to, KNIGHT);
  return move_list;
}

// --- Capture Generation ---

Move* generate_pawn_captures(const Board& board, Move* move_list) {
  const Color us = board.sideToMove;
  const Bitboard our_pawns = board.pieces[us][PAWN];
  const Bitboard their_pieces = board.occupancy[BLACK - us];
  const int promotion_rank = (us == WHITE) ? 7 : 0;

  Bitboard pawns = our_pawns;
  while (pawns) {
    const Square from = Bitboards::pop_lsb(pawns);
    Bitboard attacks = Bitboards::pawn_attacks(from, us, their_pieces);
    while (attacks) {
      const Square to = Bitboards::pop_lsb(attacks);
      if (Bitboards::rank_of(to) == promotion_rank) {
        move_list = add_promotions(from, to, move_list);
      } else {
        *move_list++ = Move(from, to);
      }
    }
  }

  if (board.enPassant != NO_SQUARE) {
    Bitboard attackers = Bitboards::pawn_attacks_mask(
                             board.enPassant, static_cast<Color>(BLACK - us)) &
                         our_pawns;
    while (attackers) {
      const Square from = Bitboards::pop_lsb(attackers);
      *move_list++ = Move(from, board.enPassant, EN_PASSANT);
    }
  }
  return move_list;
}

Move* generate_piece_captures(const Board& board, Move* move_list, Piece pt) {
  const Color us = board.sideToMove;
  const Bitboard their_pieces = board.occupancy[BLACK - us];
  Bitboard pieces = board.pieces[us][pt];
  while (pieces) {
    const Square from = Bitboards::pop_lsb(pieces);
    Bitboard attacks = 0;
    switch (pt) {
      case KNIGHT: attacks = Bitboards::knight_attacks(from); break;
      case BISHOP: attacks = Bitboards::bishop_attacks(from, board.allPieces); break;
      case ROOK: attacks = Bitboards::rook_attacks(from, board.allPieces); break;
      case QUEEN: attacks = Bitboards::queen_attacks(from, board.allPieces); break;
      case KING: attacks = Bitboards::king_attacks(from); break;
      default: break;
    }
    attacks &= their_pieces;
    while (attacks) {
      const Square to = Bitboards::pop_lsb(attacks);
      *move_list++ = Move(from, to);
    }
  }
  return move_list;
}

// --- Quiet Move Generation ---

Move* generate_pawn_quiet_moves(const Board& board, Move* move_list) {
  const Color us = board.sideToMove;
  const Bitboard our_pawns = board.pieces[us][PAWN];
  const Bitboard empty = ~board.allPieces;
  const int promotion_rank = (us == WHITE) ? 7 : 0;

  Bitboard pawns = our_pawns;
  while (pawns) {
    const Square from = Bitboards::pop_lsb(pawns);
    Bitboard pushes = Bitboards::pawn_moves(from, us, empty);
    while (pushes) {
      const Square to = Bitboards::pop_lsb(pushes);
      if (Bitboards::rank_of(to) == promotion_rank) {
        move_list = add_promotions(from, to, move_list);
      } else {
        *move_list++ = Move(from, to);
      }
    }
  }
  return move_list;
}

Move* generate_piece_quiet_moves(const Board& board, Move* move_list,
                                 Piece pt) {
  const Color us = board.sideToMove;
  const Bitboard empty = ~board.allPieces;
  Bitboard pieces = board.pieces[us][pt];
  while (pieces) {
    const Square from = Bitboards::pop_lsb(pieces);
    Bitboard attacks = 0;
    switch (pt) {
      case KNIGHT:
        attacks = Bitboards::knight_attacks(from); break;
      case BISHOP: attacks = Bitboards::bishop_attacks(from, board.allPieces); break;
      case ROOK: attacks = Bitboards::rook_attacks(from, board.allPieces); break;
      case QUEEN: attacks = Bitboards::queen_attacks(from, board.allPieces); break;
      case KING: attacks = Bitboards::king_attacks(from); break;
      default: break;
    }
    attacks &= empty;
    while (attacks) {
      const Square to = Bitboards::pop_lsb(attacks);
      *move_list++ = Move(from, to);
    }
  }
  return move_list;
}

Move* generate_castling_moves(const Board& board, Move* move_list) {
  const Color us = board.sideToMove;
  if (board.is_in_check(us)) return move_list;

  const Bitboard all = board.allPieces;
  const Color them = (us == WHITE) ? BLACK : WHITE;

  if (us == WHITE) {
    if (board.castling.whiteKingside && (all & Bitboards::WK_EMPTY) == 0 &&
        !board.attacks_to(F1, them) && !board.attacks_to(G1, them)) {
      *move_list++ = Move(E1, G1, CASTLING);
    }
    if (board.castling.whiteQueenside && (all & Bitboards::WQ_EMPTY) == 0 &&
        !board.attacks_to(D1, them) && !board.attacks_to(C1, them)) {
      *move_list++ = Move(E1, C1, CASTLING);
    }
  } else {  // BLACK
    if (board.castling.blackKingside && (all & Bitboards::BK_EMPTY) == 0 &&
        !board.attacks_to(F8, them) && !board.attacks_to(G8, them)) {
      *move_list++ = Move(E8, G8, CASTLING);
    }
    if (board.castling.blackQueenside && (all & Bitboards::BQ_EMPTY) == 0 &&
        !board.attacks_to(D8, them) && !board.attacks_to(C8, them)) {
      *move_list++ = Move(E8, C8, CASTLING);
    }
  }
  return move_list;
}

}  // namespace

namespace MoveGen {

size_t generate_pseudolegal(const Board& board,
                            std::array<Move, MAX_MOVES>& moves) {
  Move* start = moves.begin();
  Move* end = start;

  // --- Phase 1: Captures & Promotions ---
  end = generate_pawn_captures(board, end);
  end = generate_piece_captures(board, end, KNIGHT);
  end = generate_piece_captures(board, end, BISHOP);
  end = generate_piece_captures(board, end, ROOK);
  end = generate_piece_captures(board, end, QUEEN);
  end = generate_piece_captures(board, end, KING);

  // --- Phase 2: Quiet Moves ---
  end = generate_pawn_quiet_moves(board, end);
  end = generate_piece_quiet_moves(board, end, KNIGHT);
  end = generate_piece_quiet_moves(board, end, BISHOP);
  end = generate_piece_quiet_moves(board, end, ROOK);
  end = generate_piece_quiet_moves(board, end, QUEEN);
  end = generate_piece_quiet_moves(board, end, KING);
  end = generate_castling_moves(board, end);

  return static_cast<size_t>(end - start);
}

size_t generate_all(const Board& board, std::array<Move, MAX_MOVES>& moves) {
  std::array<Move, MAX_MOVES> candidate_moves;
  auto n_candidate_moves = generate_pseudolegal(board, candidate_moves);
  size_t idx = 0;
  for (size_t i = 0; i < n_candidate_moves; ++i) {
    Move m = candidate_moves[i];
    if (board.isLegalMove(m)) [[likely]]
      moves[idx++] = m;
  }
  return idx;
}

size_t generate_captures(const Board& board,
                         std::array<Move, MAX_MOVES>& moves) {
  std::array<Move, MAX_MOVES> candidate_captures;
  Move* start = candidate_captures.begin();
  Move* end = start;

  end = generate_pawn_captures(board, end);
  end = generate_piece_captures(board, end, KNIGHT);
  end = generate_piece_captures(board, end, BISHOP);
  end = generate_piece_captures(board, end, ROOK);
  end = generate_piece_captures(board, end, QUEEN);
  end = generate_piece_captures(board, end, KING);

  size_t n_candidate_moves = static_cast<size_t>(end - start);

  size_t idx = 0;
  for (size_t i = 0; i < n_candidate_moves; ++i) {
    Move m = candidate_captures[i];
    if (board.isLegalMove(m)) [[likely]]
      moves[idx++] = m;
  }

  return idx;
}

}  // namespace MoveGen
