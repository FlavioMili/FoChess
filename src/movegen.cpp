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
  const Bitboard promotion_rank =
      us == WHITE ? Bitboards::RANK_8 : Bitboards::RANK_1;

  const auto emit_attacks = [&](Bitboard attacks, int source_delta,
                                Move* end) {
    while (attacks) {
      const Square to = Bitboards::pop_lsb(attacks);
      const Square from =
          static_cast<Square>(static_cast<int>(to) + source_delta);
      if (Bitboards::square_bb(to) & promotion_rank) {
        end = add_promotions(from, to, end);
      } else {
        *end++ = Move(from, to);
      }
    }
    return end;
  };

  if (us == WHITE) {
    move_list = emit_attacks(
        Bitboards::bb_up_left(our_pawns) & their_pieces, 9, move_list);
    move_list = emit_attacks(
        Bitboards::bb_up_right(our_pawns) & their_pieces, 7, move_list);
  } else {
    move_list = emit_attacks(
        Bitboards::bb_down_left(our_pawns) & their_pieces, -7, move_list);
    move_list = emit_attacks(
        Bitboards::bb_down_right(our_pawns) & their_pieces, -9, move_list);
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
  const Bitboard empty = ~board.allPieces;
  Bitboard single_pushes;
  Bitboard double_pushes;
  Bitboard promotion_rank;

  if (us == WHITE) {
    single_pushes = Bitboards::bb_up(board.pieces[WHITE][PAWN]) & empty;
    double_pushes =
        Bitboards::bb_up(single_pushes & Bitboards::RANK_3) & empty;
    promotion_rank = Bitboards::RANK_8;
  } else {
    single_pushes = Bitboards::bb_down(board.pieces[BLACK][PAWN]) & empty;
    double_pushes =
        Bitboards::bb_down(single_pushes & Bitboards::RANK_6) & empty;
    promotion_rank = Bitboards::RANK_1;
  }

  Bitboard pushes = single_pushes | double_pushes;
  while (pushes) {
    const Square to = Bitboards::pop_lsb(pushes);
    const Bitboard to_bb = Bitboards::square_bb(to);
    const bool is_double = (double_pushes & to_bb) != 0;
    const int delta = us == WHITE ? (is_double ? 16 : 8)
                                  : (is_double ? -16 : -8);
    const Square from =
        static_cast<Square>(static_cast<int>(to) + delta);
    if (to_bb & promotion_rank) {
      move_list = add_promotions(from, to, move_list);
    } else {
      *move_list++ = Move(from, to);
    }
  }
  return move_list;
}

Move* generate_pawn_quiet_promotions(const Board& board, Move* move_list) {
  const Color us = board.sideToMove;
  const Bitboard empty = ~board.allPieces;
  const int promotion_rank = (us == WHITE) ? 7 : 0;
  Bitboard pawns = board.pieces[us][PAWN];

  while (pawns) {
    const Square from = Bitboards::pop_lsb(pawns);
    Bitboard pushes = Bitboards::pawn_moves(from, us, empty);
    while (pushes) {
      const Square to = Bitboards::pop_lsb(pushes);
      if (Bitboards::rank_of(to) == promotion_rank)
        move_list = add_promotions(from, to, move_list);
    }
  }
  return move_list;
}

Move* generate_piece_moves(const Board& board, Move* move_list, Piece pt) {
  const Color us = board.sideToMove;
  Bitboard pieces = board.pieces[us][pt];
  while (pieces) {
    const Square from = Bitboards::pop_lsb(pieces);
    Bitboard attacks = 0;
    switch (pt) {
      case KNIGHT: attacks = Bitboards::knight_attacks(from); break;
      case BISHOP:
        attacks = Bitboards::bishop_attacks(from, board.allPieces); break;
      case ROOK:
        attacks = Bitboards::rook_attacks(from, board.allPieces); break;
      case QUEEN:
        attacks = Bitboards::queen_attacks(from, board.allPieces); break;
      case KING: attacks = Bitboards::king_attacks(from); break;
      default: break;
    }
    attacks &= ~board.occupancy[us];
    while (attacks) {
      const Square to = Bitboards::pop_lsb(attacks);
      *move_list++ = Move(from, to);
    }
  }
  return move_list;
}

Move* generate_castling_moves(const Board& board, Move* move_list) {
  const Color us = board.sideToMove;
  if (us == WHITE) {
    if (!board.castling.whiteKingside && !board.castling.whiteQueenside)
      return move_list;
  } else if (!board.castling.blackKingside &&
             !board.castling.blackQueenside) {
    return move_list;
  }

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
  auto start = moves.begin();
  Move* end = start;

  // Pawns retain their specialized capture/push handling. Other pieces emit
  // captures and quiet moves from one attack lookup instead of recomputing it.
  end = generate_pawn_captures(board, end);
  end = generate_pawn_quiet_moves(board, end);
  end = generate_piece_moves(board, end, KNIGHT);
  end = generate_piece_moves(board, end, BISHOP);
  end = generate_piece_moves(board, end, ROOK);
  end = generate_piece_moves(board, end, QUEEN);
  end = generate_piece_moves(board, end, KING);
  end = generate_castling_moves(board, end);

  return static_cast<size_t>(end - start);
}

size_t generate_all(const Board& board, std::array<Move, MAX_MOVES>& moves) {
  const size_t n_candidate_moves = generate_pseudolegal(board, moves);
  const Color us = board.sideToMove;
  const Color them = static_cast<Color>(BLACK - us);
  const Bitboard king_bb = board.pieces[us][KING];
  const Square king_sq = board.kingSq[us];
  const Bitboard bishop_rays =
      Bitboards::bishop_attacks(king_sq, board.allPieces);
  const Bitboard rook_rays = Bitboards::rook_attacks(king_sq, board.allPieces);
  const bool in_check =
      (bishop_rays &
       (board.pieces[them][BISHOP] | board.pieces[them][QUEEN])) != 0 ||
      (rook_rays &
       (board.pieces[them][ROOK] | board.pieces[them][QUEEN])) != 0 ||
      (Bitboards::knight_attacks(king_sq) & board.pieces[them][KNIGHT]) != 0 ||
      (Bitboards::pawn_attacks_mask(king_sq, us) &
       board.pieces[them][PAWN]) != 0 ||
      (Bitboards::king_attacks(king_sq) & board.pieces[them][KING]) != 0;

  Bitboard pinned = 0;
  if (!in_check) [[likely]] {
    // Remove our first blocker on each ray, then find enemy sliders revealed
    // behind it. Looking back from each pinner identifies the exact blocker;
    // ordinary adjacent king pawns are therefore not needlessly revalidated.
    const Bitboard diagonal_blockers = bishop_rays & board.occupancy[us];
    Bitboard diagonal_pinners =
        Bitboards::bishop_attacks(
            king_sq, board.allPieces ^ diagonal_blockers) &
        (board.pieces[them][BISHOP] | board.pieces[them][QUEEN]);
    while (diagonal_pinners) {
      const Square pinner = Bitboards::pop_lsb(diagonal_pinners);
      pinned |= diagonal_blockers &
                Bitboards::bishop_attacks(pinner, board.allPieces);
    }

    const Bitboard straight_blockers = rook_rays & board.occupancy[us];
    Bitboard straight_pinners =
        Bitboards::rook_attacks(king_sq,
                                board.allPieces ^ straight_blockers) &
        (board.pieces[them][ROOK] | board.pieces[them][QUEEN]);
    while (straight_pinners) {
      const Square pinner = Bitboards::pop_lsb(straight_pinners);
      pinned |= straight_blockers &
                Bitboards::rook_attacks(pinner, board.allPieces);
    }
  }

  size_t idx = 0;
  for (size_t i = 0; i < n_candidate_moves; ++i) {
    const Move m = moves[i];
    const Bitboard from_bb = Bitboards::square_bb(m.from_sq());
    const bool needs_legality_test =
        in_check || (from_bb & (king_bb | pinned)) != 0 ||
        m.type() == MoveType::EN_PASSANT;
    if (!needs_legality_test || board.isLegalMove(m)) moves[idx++] = m;
  }
  return idx;
}

size_t generate_captures(const Board& board,
                         std::array<Move, MAX_MOVES>& moves) {
  auto start = moves.begin();
  Move* end = start;

  end = generate_pawn_captures(board, end);
  end = generate_piece_captures(board, end, KNIGHT);
  end = generate_piece_captures(board, end, BISHOP);
  end = generate_piece_captures(board, end, ROOK);
  end = generate_piece_captures(board, end, QUEEN);
  end = generate_piece_captures(board, end, KING);

  auto n_candidate_moves = static_cast<size_t>(end - start);

  size_t idx = 0;
  for (size_t i = 0; i < n_candidate_moves; ++i) {
    const Move m = moves[i];
    if (board.isLegalMove(m)) [[likely]]
      moves[idx++] = m;
  }

  return idx;
}

size_t generate_noisy(const Board& board,
                      std::array<Move, MAX_MOVES>& moves) {
  Move* end = moves.data();

  end = generate_pawn_captures(board, end);
  end = generate_piece_captures(board, end, KNIGHT);
  end = generate_piece_captures(board, end, BISHOP);
  end = generate_piece_captures(board, end, ROOK);
  end = generate_piece_captures(board, end, QUEEN);
  end = generate_piece_captures(board, end, KING);
  end = generate_pawn_quiet_promotions(board, end);

  size_t count = 0;
  for (Move* candidate = moves.data(); candidate != end; ++candidate) {
    if (board.isLegalMove(*candidate)) moves[count++] = *candidate;
  }
  return count;
}

bool has_legal_move(const Board& board) {
  std::array<Move, MAX_MOVES> candidates;
  const size_t count = generate_pseudolegal(board, candidates);
  for (size_t i = 0; i < count; ++i) {
    if (board.isLegalMove(candidates[i])) return true;
  }
  return false;
}

}  // namespace MoveGen
