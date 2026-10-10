// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2026 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include "board.h"
#include "types.h"
  
namespace FoChess {

namespace {

constexpr int MAX_PHASE = 24;
constexpr std::array<int, 6> middlegame_values = {180, 550, 630,
                                                  900, 1620, 0};
constexpr std::array<int, 6> endgame_values = {200, 520, 600,
                                               940, 1600, 0};
constexpr std::array<int, 6> phase_weights = {0, 1, 1, 2, 4, 0};

static constexpr std::array<int, 64> pawn_table = {
    990,  990,  990,  990,  990,  990,  990,  990,
    5, 10, 10,20,20, 10, 10,  5,
    5, 5,10,  0,  0,10, 5,  5,
    0,  0,  0, 20, 20,  0,  0,  0,
    5,  5, 10,25,25, 10,  5,  5,
    10, 20, 20,20,20, 20, 20, 10,
    30, 30, 20,19,19, 20, 30, 30,
    0,  0,  0,  0,  0,  0,  0,  0
};

static constexpr std::array<int, 64> knight_table = {
   -50,-40,-30,-30,-30,-30,-40,-50,
   -40,-20,  0,  0,  0,  0,-20,-40,
   -30,  0, 10,15,15,10,  0,-30,
   -30,  5, 15,20,20,15,  5,-30,
   -30,  0, 15,20,20,15,  0,-30,
   -30,  5, 10,15,15,10,  5,-30,
   -40,-20,  0,  5,  5,  0,-20,-40,
   -50,-40,-30,-30,-30,-30,-40,-50
};

static constexpr std::array<int, 64> bishop_table = {
   -20,-10,-10,-10,-10,-10,-10,-20,
   -10,  0,  0,  0,  0,  0,  0,-10,
   -10,  0,  5,10,10, 5,  0,-10,
   -10,  5,  5,10,10, 5,  5,-10,
   -10,  0, 10,10,10,10,  0,-10,
   -10, 10, 10,10,10,10, 10,-10,
   -10,  9,  0,  0,  0,  0,  9,-10,
   -20,-10,-10,-10,-10,-10,-10,-20
};

static constexpr std::array<int, 64> rook_table = {
     0,  1,  1,  1,  1,  1,  1,  0,
     5, 10, 10, 10, 10, 10, 10,  5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
     0, 10, 10, 10, 10, 10, 10,  0,
     0,  0,  0,  5,  5,  0,  0,  0
};

static constexpr std::array<int, 64> queen_table = {
   -20,-10,-10, -5, -5,-10,-10,-20,
   -10,  1,  2,  1,  1,  2,  1,-10,
   -10,  0,  2,  2,  2,  2,  1,-10,
    -5,  0,  3,  4,  4,  3,  1, -5,
     0,  0,  3,  5,  5,  3,  1, -5,
   -10,  5,  5,  5,  5,  5,  1,-10,
   -10,  0,  5,  0,  0,  0,  0,-10,
   -20,-10,-10, -5, -5,-10,-10,-20
};

static constexpr std::array<int, 64> king_middlegame_table = {
   -30,-40,-40,-50,-50,-40,-40,-30,
   -30,-40,-40,-50,-50,-40,-40,-30,
   -30,-40,-40,-50,-50,-40,-40,-30,
   -30,-40,-40,-50,-50,-40,-40,-30,
   -20,-30,-30,-40,-40,-30,-30,-20,
   -10,-20,-20,-20,-20,-20,-20,-10,
    20, 20,  0,  0,  0,  0, 20, 20,
    20, 30, 10,  0,  0, 10, 30, 20
};

static constexpr std::array<int, 64> king_endgame_table = {
   -50,-40,-30,-20,-20,-30,-40,-50,
   -30,-20,-10,  0,  0,-10,-20,-30,
   -30,-10, 20, 30, 30, 20,-10,-30,
   -30,-10, 30, 40, 40, 30,-10,-30,
   -30,-10, 30, 40, 40, 30,-10,-30,
   -30,-10, 20, 30, 30, 20,-10,-30,
   -30,-30,  0,  0,  0,  0,-30,-30,
   -50,-30,-30,-30,-30,-30,-30,-50
};

constexpr std::array<const int*, 6> middlegame_tables = {
    pawn_table.data(), knight_table.data(), bishop_table.data(),
    rook_table.data(), queen_table.data(), king_middlegame_table.data()};

constexpr std::array<const int*, 6> endgame_tables = {
    pawn_table.data(), knight_table.data(), bishop_table.data(),
    rook_table.data(), queen_table.data(), king_endgame_table.data()};

constexpr std::array<Bitboard, 8> file_masks = {
    Bitboards::FILE_A, Bitboards::FILE_B, Bitboards::FILE_C, Bitboards::FILE_D,
    Bitboards::FILE_E, Bitboards::FILE_F, Bitboards::FILE_G, Bitboards::FILE_H};

constexpr std::array<std::array<Bitboard, 64>, 2> make_passed_pawn_masks() {
  std::array<std::array<Bitboard, 64>, 2> masks{};
  for (int color = WHITE; color <= BLACK; ++color) {
    for (int square = A8; square <= H1; ++square) {
      const auto pawn_square = static_cast<Square>(square);
      const int pawn_rank = Bitboards::rank_of(pawn_square);
      const int pawn_file = Bitboards::file_of(pawn_square);
      Bitboard mask = 0;
      for (int target = A8; target <= H1; ++target) {
        const auto target_square = static_cast<Square>(target);
        const int target_rank = Bitboards::rank_of(target_square);
        const int target_file = Bitboards::file_of(target_square);
        const bool adjacent_file =
            target_file >= pawn_file - 1 && target_file <= pawn_file + 1;
        const bool ahead = color == WHITE ? target_rank > pawn_rank
                                          : target_rank < pawn_rank;
        if (adjacent_file && ahead)
          mask |= Bitboards::square_bb(target_square);
      }
      masks[static_cast<size_t>(color)][static_cast<size_t>(square)] = mask;
    }
  }
  return masks;
}

constexpr auto passed_pawn_masks = make_passed_pawn_masks();
constexpr std::array<int, 8> passed_middlegame_bonus = {0, 5, 10, 20,
                                                        35, 60, 100, 0};
constexpr std::array<int, 8> passed_endgame_bonus = {0, 10, 20, 40,
                                                     70, 120, 200, 0};

void add_pawn_structure(const Board& board, Color color, int sign,
                        int& middlegame, int& endgame) {
  const Bitboard pawns = board.pieces[color][PAWN];
  const Bitboard enemy_pawns = board.pieces[BLACK - color][PAWN];

  for (int file = 0; file < 8; ++file) {
    const int count = Bitboards::popcount(
        pawns & file_masks[static_cast<size_t>(file)]);
    if (count > 1) {
      middlegame += sign * -20 * (count - 1);
      endgame += sign * -30 * (count - 1);
    }

    Bitboard neighbours = 0;
    if (file > 0) neighbours |= file_masks[static_cast<size_t>(file - 1)];
    if (file < 7) neighbours |= file_masks[static_cast<size_t>(file + 1)];
    if (count != 0 && (pawns & neighbours) == 0) {
      middlegame += sign * -15 * count;
      endgame += sign * -25 * count;
    }
  }

  Bitboard remaining = pawns;
  while (remaining != 0) {
    const Square square = Bitboards::pop_lsb(remaining);
    if ((enemy_pawns & passed_pawn_masks[color][square]) != 0) continue;
    const int rank = Bitboards::rank_of(square);
    const int advancement = color == WHITE ? rank : 7 - rank;
    middlegame +=
        sign * passed_middlegame_bonus[static_cast<size_t>(advancement)];
    endgame += sign * passed_endgame_bonus[static_cast<size_t>(advancement)];
  }
}

void add_mobility(const Board& board, Color color, int sign, int& middlegame,
                  int& endgame, Bitboard enemy_king_ring,
                  int& attack_units, int& attacker_count) {
  const Bitboard available = ~board.occupancy[color];
  Bitboard pieces = board.pieces[color][KNIGHT];
  while (pieces != 0) {
    const Square square = Bitboards::pop_lsb(pieces);
    const Bitboard attacks = Bitboards::knight_attacks(square);
    const int mobility = Bitboards::popcount(attacks & available);
    middlegame += sign * 4 * mobility;
    endgame += sign * 4 * mobility;
    const int ring_hits = Bitboards::popcount(attacks & enemy_king_ring);
    if (ring_hits != 0) {
      ++attacker_count;
      attack_units += 2 * ring_hits;
    }
  }

  pieces = board.pieces[color][BISHOP];
  while (pieces != 0) {
    const Square square = Bitboards::pop_lsb(pieces);
    const Bitboard attacks =
        Bitboards::bishop_attacks(square, board.allPieces);
    const int mobility = Bitboards::popcount(attacks & available);
    middlegame += sign * 4 * mobility;
    endgame += sign * 5 * mobility;
    const int ring_hits = Bitboards::popcount(attacks & enemy_king_ring);
    if (ring_hits != 0) {
      ++attacker_count;
      attack_units += 2 * ring_hits;
    }
  }

  pieces = board.pieces[color][ROOK];
  while (pieces != 0) {
    const Square square = Bitboards::pop_lsb(pieces);
    const Bitboard attacks = Bitboards::rook_attacks(square, board.allPieces);
    const int mobility = Bitboards::popcount(attacks & available);
    middlegame += sign * 2 * mobility;
    endgame += sign * 4 * mobility;
    const int ring_hits = Bitboards::popcount(attacks & enemy_king_ring);
    if (ring_hits != 0) {
      ++attacker_count;
      attack_units += 3 * ring_hits;
    }
  }

  pieces = board.pieces[color][QUEEN];
  while (pieces != 0) {
    const Square square = Bitboards::pop_lsb(pieces);
    const Bitboard attacks =
        Bitboards::bishop_attacks(square, board.allPieces) |
        Bitboards::rook_attacks(square, board.allPieces);
    const int mobility = Bitboards::popcount(attacks & available);
    middlegame += sign * mobility;
    endgame += sign * 2 * mobility;
    const int ring_hits = Bitboards::popcount(attacks & enemy_king_ring);
    if (ring_hits != 0) {
      ++attacker_count;
      attack_units += 5 * ring_hits;
    }
  }
}

Bitboard pawn_attack_map(Bitboard pawns, Color color) {
  return color == WHITE
             ? Bitboards::bb_up_left(pawns) | Bitboards::bb_up_right(pawns)
             : Bitboards::bb_down_left(pawns) |
                   Bitboards::bb_down_right(pawns);
}

int king_shelter(const Board& board, Color color) {
  const Square king = board.kingSq[color];
  const int king_rank = Bitboards::rank_of(king);
  const int king_file = Bitboards::file_of(king);
  const int forward = color == WHITE ? 1 : -1;
  const Bitboard friendly_pawns = board.pieces[color][PAWN];
  const Color enemy = static_cast<Color>(BLACK - color);

  int safety = 0;
  for (int file = std::max(0, king_file - 1);
       file <= std::min(7, king_file + 1); ++file) {
    const int first_rank = king_rank + forward;
    const int second_rank = king_rank + 2 * forward;
    if (first_rank >= 0 && first_rank < 8) {
      const auto square = static_cast<Square>((7 - first_rank) * 8 + file);
      if ((friendly_pawns & Bitboards::square_bb(square)) != 0) safety += 12;
    }
    if (second_rank >= 0 && second_rank < 8) {
      const auto square = static_cast<Square>((7 - second_rank) * 8 + file);
      if ((friendly_pawns & Bitboards::square_bb(square)) != 0) safety += 5;
    }

    const Bitboard file_mask = file_masks[static_cast<size_t>(file)];
    if ((friendly_pawns & file_mask) == 0) {
      safety -= 10;
      if ((board.pieces[enemy][PAWN] & file_mask) == 0) safety -= 6;
    }
  }

  return safety;
}

int king_attack_penalty(int attack_units, int attacker_count) {
  return attacker_count >= 2 ? attack_units * attacker_count : attack_units;
}

struct PawnKingEvaluation {
  int middlegame = 0;
  int endgame = 0;
  uint8_t white_attack_units = 0;
  uint8_t black_attack_units = 0;
};

struct PawnKingCacheEntry {
  Bitboard white_pawns = 0;
  Bitboard black_pawns = 0;
  int middlegame = 0;
  int endgame = 0;
  uint16_t king_key = 0;
  uint8_t white_attack_units = 0;
  uint8_t black_attack_units = 0;
  bool valid = false;
};

constexpr size_t PAWN_KING_CACHE_SIZE = 2048;
static_assert((PAWN_KING_CACHE_SIZE & (PAWN_KING_CACHE_SIZE - 1)) == 0);
constexpr int PAWN_KING_CACHE_BITS =
    std::countr_zero(PAWN_KING_CACHE_SIZE);
thread_local std::array<PawnKingCacheEntry, PAWN_KING_CACHE_SIZE>
    pawn_king_cache{};

PawnKingEvaluation evaluate_pawns_and_shelter(const Board& board) {
  const Bitboard white_pawns = board.pieces[WHITE][PAWN];
  const Bitboard black_pawns = board.pieces[BLACK][PAWN];
  const uint16_t king_key = static_cast<uint16_t>(board.kingSq[WHITE]) |
                            static_cast<uint16_t>(
                                static_cast<uint16_t>(board.kingSq[BLACK])
                                << 7);
  uint64_t mixed = white_pawns ^ (black_pawns << 23) ^
                   (black_pawns >> 41) ^
                   (static_cast<uint64_t>(king_key) *
                    0xD6E8FEB86659FD93ULL);
  mixed ^= mixed >> 32;
  mixed *= 0x9E3779B97F4A7C15ULL;
  const size_t index =
      static_cast<size_t>(mixed >> (64 - PAWN_KING_CACHE_BITS));
  PawnKingCacheEntry& cached = pawn_king_cache[index];
  if (cached.valid && cached.white_pawns == white_pawns &&
      cached.black_pawns == black_pawns && cached.king_key == king_key) {
    return {cached.middlegame, cached.endgame, cached.white_attack_units,
            cached.black_attack_units};
  }

  PawnKingEvaluation result;
  for (size_t color = WHITE; color <= BLACK; ++color) {
    const int sign = color == WHITE ? 1 : -1;
    Bitboard pawns = board.pieces[color][PAWN];
    while (pawns != 0) {
      const Square square = Bitboards::pop_lsb(pawns);
      size_t table_square = static_cast<size_t>(square);
      if (color == BLACK) table_square ^= 56U;
      result.middlegame +=
          sign * (middlegame_values[PAWN] + pawn_table[table_square]);
      result.endgame +=
          sign * (endgame_values[PAWN] + pawn_table[table_square]);
    }
  }

  add_pawn_structure(board, WHITE, 1, result.middlegame, result.endgame);
  add_pawn_structure(board, BLACK, -1, result.middlegame, result.endgame);
  result.middlegame +=
      king_shelter(board, WHITE) - king_shelter(board, BLACK);

  const Bitboard white_ring = Bitboards::king_attacks(board.kingSq[WHITE]);
  const Bitboard black_ring = Bitboards::king_attacks(board.kingSq[BLACK]);
  result.white_attack_units = static_cast<uint8_t>(Bitboards::popcount(
      pawn_attack_map(white_pawns, WHITE) & black_ring));
  result.black_attack_units = static_cast<uint8_t>(Bitboards::popcount(
      pawn_attack_map(black_pawns, BLACK) & white_ring));

  cached = {white_pawns,
            black_pawns,
            result.middlegame,
            result.endgame,
            king_key,
            result.white_attack_units,
            result.black_attack_units,
            true};
  return result;
}

}  // namespace

int bland_evaluate(const Board& board) {
  const PawnKingEvaluation pawn_king = evaluate_pawns_and_shelter(board);
  int middlegame = pawn_king.middlegame;
  int endgame = pawn_king.endgame;
  int phase = 0;

  for (size_t piece = KNIGHT; piece <= KING; ++piece) {
    for (size_t color = WHITE; color <= BLACK; ++color) {
      const int sign = color == WHITE ? 1 : -1;
      Bitboard pieces = board.pieces[color][piece];
      phase += phase_weights[piece] * Bitboards::popcount(pieces);
      while (pieces != 0) {
        const Square square = Bitboards::pop_lsb(pieces);
        size_t table_square = static_cast<size_t>(square);
        if (color == BLACK) table_square ^= 56U;
        middlegame += sign *
            (middlegame_values[piece] +
             middlegame_tables[piece][table_square]);
        endgame += sign *
            (endgame_values[piece] + endgame_tables[piece][table_square]);
      }
    }
  }

  if (Bitboards::popcount(board.pieces[WHITE][BISHOP]) >= 2) {
    middlegame += 30;
    endgame += 50;
  }
  if (Bitboards::popcount(board.pieces[BLACK][BISHOP]) >= 2) {
    middlegame -= 30;
    endgame -= 50;
  }

  const Bitboard white_ring = Bitboards::king_attacks(board.kingSq[WHITE]);
  const Bitboard black_ring = Bitboards::king_attacks(board.kingSq[BLACK]);
  int white_attack_units = pawn_king.white_attack_units;
  int black_attack_units = pawn_king.black_attack_units;
  int white_attackers = white_attack_units != 0;
  int black_attackers = black_attack_units != 0;

  add_mobility(board, WHITE, 1, middlegame, endgame, black_ring,
               white_attack_units, white_attackers);
  add_mobility(board, BLACK, -1, middlegame, endgame, white_ring,
               black_attack_units, black_attackers);
  middlegame += king_attack_penalty(white_attack_units, white_attackers) -
                king_attack_penalty(black_attack_units, black_attackers);

  phase = std::min(phase, MAX_PHASE);
  const int score =
      (middlegame * phase + endgame * (MAX_PHASE - phase)) / MAX_PHASE;
  return board.sideToMove == WHITE ? score : -score;
}

}  // namespace FoChess
