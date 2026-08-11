// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#include "search.h"

#include <algorithm>
#include <array>
#include <cstddef>

#include "board.h"
#include "evaluate.h"
#include "movegen.h"
#include "tt.h"

namespace FoChess {

namespace {

constexpr int TT_MATE_THRESHOLD = MATE_SCORE - 256;
constexpr int MAX_SEARCH_PLY = 128;
constexpr std::array<int, 6> ORDER_PIECE_VALUES = {100, 320, 330,
                                                   500, 900, 20000};
constexpr std::array<int, 6> DELTA_PIECE_VALUES = {180, 550, 630,
                                                   900, 1340, 20000};
constexpr int DELTA_MARGIN = 200;

std::array<std::array<Move, 2>, MAX_SEARCH_PLY> killer_moves;
std::array<std::array<std::array<int, 64>, 64>, 2> history_scores{};

bool is_capture(const Board& board, const Move& move) {
  return move.type() == EN_PASSANT ||
         (Bitboards::square_bb(move.to_sq()) &
          board.occupancy[BLACK - board.sideToMove]) != 0;
}

bool is_insufficient_material(const Board& board) {
  const Bitboard pawns_rooks_queens =
      board.pieces[WHITE][PAWN] | board.pieces[BLACK][PAWN] |
      board.pieces[WHITE][ROOK] | board.pieces[BLACK][ROOK] |
      board.pieces[WHITE][QUEEN] | board.pieces[BLACK][QUEEN];
  if (pawns_rooks_queens != 0) return false;

  const Bitboard knights =
      board.pieces[WHITE][KNIGHT] | board.pieces[BLACK][KNIGHT];
  Bitboard bishops =
      board.pieces[WHITE][BISHOP] | board.pieces[BLACK][BISHOP];
  const int minor_count = Bitboards::popcount(knights | bishops);
  if (minor_count <= 1) return true;
  if (knights != 0) return false;

  // With bishops only, the position is dead if every bishop is confined to
  // the same square color.
  int square_color = -1;
  while (bishops) {
    const Square square = Bitboards::pop_lsb(bishops);
    const int color = (Bitboards::rank_of(square) +
                       Bitboards::file_of(square)) & 1;
    if (square_color == -1)
      square_color = color;
    else if (square_color != color)
      return false;
  }
  return true;
}

bool rule_draw(const Board& board) {
  return board.halfMoveClock >= 100 || is_insufficient_material(board);
}

int move_order_score(const Board& board, const Move& move,
                     const Move& tt_move, int ply) {
  if (move == tt_move) return 10'000'000;

  int score = 0;
  if (move.type() == PROMOTION)
    score += 8'000'000 + ORDER_PIECE_VALUES[move.promotion_type()];

  if (is_capture(board, move)) {
    const Piece victim = move.type() == EN_PASSANT ? PAWN
                                                    : board.piece_on(move.to_sq());
    const Piece attacker = board.piece_on(move.from_sq());
    if (victim != NO_PIECE && attacker != NO_PIECE)
      score += 4'000'000 + 16 * ORDER_PIECE_VALUES[victim] -
               ORDER_PIECE_VALUES[attacker];
    return score;
  }

  if (ply < MAX_SEARCH_PLY) {
    if (move == killer_moves[static_cast<size_t>(ply)][0]) return 3'000'000;
    if (move == killer_moves[static_cast<size_t>(ply)][1]) return 2'900'000;
  }

  return score + history_scores[board.sideToMove][move.from_sq()][move.to_sq()];
}

void order_moves(const Board& board, std::array<Move, MAX_MOVES>& moves,
                 size_t count, const Move& tt_move, int ply) {
  struct ScoredMove {
    int score;
    Move move;
    uint16_t original_index;
  };
  static_assert(sizeof(ScoredMove) == 8);
  std::array<ScoredMove, MAX_MOVES> scored;
  for (size_t i = 0; i < count; ++i) {
    scored[i] = {move_order_score(board, moves[i], tt_move, ply), moves[i],
                 static_cast<uint16_t>(i)};
  }

  std::sort(scored.begin(), scored.begin() + static_cast<ptrdiff_t>(count),
            [](const ScoredMove& lhs, const ScoredMove& rhs) {
              if (lhs.score != rhs.score) return lhs.score > rhs.score;
              return lhs.original_index < rhs.original_index;
            });
  for (size_t i = 0; i < count; ++i) moves[i] = scored[i].move;
}

void record_quiet_cutoff(const Board& board, const Move& move, int depth,
                         int ply) {
  if (is_capture(board, move) || move.type() == PROMOTION) return;

  if (ply < MAX_SEARCH_PLY) {
    auto& killers = killer_moves[static_cast<size_t>(ply)];
    if (move != killers[0]) {
      killers[1] = killers[0];
      killers[0] = move;
    }
  }

  int& history = history_scores[board.sideToMove][move.from_sq()][move.to_sq()];
  history = std::min(1'000'000, history + depth * depth);
}

int score_to_tt(int score, int ply) {
  if (score >= TT_MATE_THRESHOLD) return score + ply;
  if (score <= -TT_MATE_THRESHOLD) return score - ply;
  return score;
}

int score_from_tt(int score, int ply) {
  if (score >= TT_MATE_THRESHOLD) return score - ply;
  if (score <= -TT_MATE_THRESHOLD) return score + ply;
  return score;
}

int quiescence_impl(Board& board, int alpha, int beta, int ply) {
  if ((g_search_stats.node_count.load(std::memory_order_relaxed) & 2047) == 0 &&
      should_stop_search())
    return 0;

  g_search_stats.node_count.fetch_add(1, std::memory_order_relaxed);
  if (ply >= MAX_SEARCH_PLY) return bland_evaluate(board);

  const bool in_check = board.is_in_check(board.sideToMove);
  if (rule_draw(board)) {
    if (!in_check || board.halfMoveClock < 100) return 0;

    std::array<Move, MAX_MOVES> evasions;
    if (MoveGen::generate_all(board, evasions) == 0)
      return -MATE_SCORE + ply;
    return 0;
  }
  std::array<Move, MAX_MOVES> moves;
  const size_t count = in_check ? MoveGen::generate_all(board, moves)
                                : MoveGen::generate_noisy(board, moves);

  if (in_check && count == 0) return -MATE_SCORE + ply;
  if (!in_check && count == 0 && !MoveGen::has_legal_move(board)) return 0;

  int stand_pat = -INF_SCORE;
  if (!in_check) {
    stand_pat = bland_evaluate(board);
    if (stand_pat >= beta) return beta;
    if (stand_pat > alpha) alpha = stand_pat;
  }

  order_moves(board, moves, count, Move::none(), ply);

  for (size_t i = 0; i < count; ++i) {
    Board child = board;
    child.makeMove(moves[i]);

    if (!in_check && moves[i].type() != PROMOTION &&
        is_capture(board, moves[i])) {
      const Piece victim = moves[i].type() == EN_PASSANT
                               ? PAWN
                               : board.piece_on(moves[i].to_sq());
      if (victim != NO_PIECE &&
          stand_pat + DELTA_PIECE_VALUES[victim] + DELTA_MARGIN < alpha &&
          !child.is_in_check(child.sideToMove)) {
        continue;
      }
    }

    const int score = -quiescence_impl(child, -beta, -alpha, ply + 1);

    if (g_search_state.should_stop.load(std::memory_order_relaxed)) return 0;
    if (score >= beta) return beta;
    if (score > alpha) alpha = score;
  }
  return alpha;
}

}  // namespace

void reset_search_heuristics() {
  for (auto& killers : killer_moves)
    killers = {Move::none(), Move::none()};
  for (auto& color : history_scores)
    for (auto& from : color) from.fill(0);
}

int alpha_beta_pruning(int depth, Board& board, int alpha, int beta, int ply) {
  if ((g_search_stats.node_count.load(std::memory_order_relaxed) & 2047) == 0 &&
      should_stop_search()) {
    return 0;
  }

  if (depth == 0) return quiescence_search(board, alpha, beta, ply);

  if (rule_draw(board)) {
    if (!board.is_in_check(board.sideToMove) || board.halfMoveClock < 100)
      return 0;
    std::array<Move, MAX_MOVES> evasions;
    if (MoveGen::generate_all(board, evasions) == 0)
      return -MATE_SCORE + ply;
    return 0;
  }

  FoChess::g_search_stats.node_count.fetch_add(1, std::memory_order_relaxed);

  std::array<Move, MAX_MOVES> moves;
  size_t n = MoveGen::generate_all(board, moves);

  if (n == 0) [[unlikely]] {
    if (board.is_in_check(board.sideToMove)) return -MATE_SCORE + ply;
    return 0;
  }

  int best = -INF_SCORE;
  Move best_move = moves[0];
  const bool allow_lmr =
      depth >= 3 && !board.is_in_check(board.sideToMove);

  order_moves(board, moves, n, Move::none(), ply);

  for (size_t i = 0; i < n; i++) {
    Board tmp = board;
    tmp.makeMove(moves[i]);
    int score;
    if (i == 0) {
      score =
          -alpha_beta_pruning(depth - 1, tmp, -beta, -alpha, ply + 1);
    } else {
      const bool reduce =
          allow_lmr && i >= 3 && !is_capture(board, moves[i]) &&
          moves[i].type() != PROMOTION;
      score = -alpha_beta_pruning(depth - 1 - static_cast<int>(reduce), tmp,
                                  -alpha - 1, -alpha, ply + 1);
      if (reduce &&
          !g_search_state.should_stop.load(std::memory_order_relaxed) &&
          score > alpha) {
        score = -alpha_beta_pruning(depth - 1, tmp, -alpha - 1, -alpha,
                                    ply + 1);
      }
      if (!g_search_state.should_stop.load(std::memory_order_relaxed) &&
          score > alpha && score < beta) {
        score =
            -alpha_beta_pruning(depth - 1, tmp, -beta, -alpha, ply + 1);
      }
    }

    if (g_search_state.should_stop.load(std::memory_order_relaxed)) return 0;

    if (score > best) {
      best = score;
      best_move = moves[i];
    }

    alpha = std::max(alpha, score);
    if (alpha >= beta) {
      record_quiet_cutoff(board, moves[i], depth, ply);
      break;
    }
  }

  if (ply == 0 &&
      !g_search_state.should_stop.load(std::memory_order_relaxed)) {
    g_search_stats.best_move.store(best_move, std::memory_order_relaxed);
    g_search_stats.best_root_score.store(best, std::memory_order_relaxed);
  }

  return best;
}

int quiescence_search(Board& board, int alpha, int beta, int ply) {
  return quiescence_impl(board, alpha, beta, ply);
}

int alpha_beta_pruning(int depth, Board& board, TranspositionTable& tt,
                       int alpha, int beta, int ply) {
  if ((g_search_stats.node_count.load(std::memory_order_relaxed) & 2047) == 0 &&
      should_stop_search())
    return 0;

  if (depth == 0) return quiescence_search(board, tt, alpha, beta, ply);

  if (rule_draw(board)) {
    if (!board.is_in_check(board.sideToMove) || board.halfMoveClock < 100)
      return 0;
    std::array<Move, MAX_MOVES> evasions;
    if (MoveGen::generate_all(board, evasions) == 0)
      return -MATE_SCORE + ply;
    return 0;
  }

  const uint64_t hash_key = board.hash;
  TTEntry* tte = tt.probe(hash_key);

  Move tt_move = Move::none();
  if (tte) {
    tt_move = tte->best_move;
    const bool valid_tt_move =
        board.moveExists(tt_move) && board.isLegalMove(tt_move);
    if (tte->depth >= depth && valid_tt_move) {
      const int tt_score = score_from_tt(tte->score, ply);
      if (tte->flag == TT_EXACT ||
          (tte->flag == TT_ALPHA && tt_score <= alpha) ||
          (tte->flag == TT_BETA && tt_score >= beta)) {
        if (ply == 0)
          g_search_stats.best_move.store(tt_move, std::memory_order_relaxed);
        return tt_score;
      }
    }
  }

  FoChess::g_search_stats.node_count.fetch_add(1, std::memory_order_relaxed);

  std::array<Move, MAX_MOVES> moves;
  size_t n = MoveGen::generate_all(board, moves);
  if (n == 0)
    return (board.is_in_check(board.sideToMove)) ? -MATE_SCORE + ply : 0;

  order_moves(board, moves, n, tt_move, ply);

  int best = -INF_SCORE;
  Move best_move = Move::none();
  const bool allow_lmr =
      depth >= 3 && !board.is_in_check(board.sideToMove);

  const int original_alpha = alpha;

  for (size_t i = 0; i < n; ++i) {
    Board tmp = board;
    tmp.makeMove(moves[i]);

    int score;
    if (i == 0) {
      score = -alpha_beta_pruning(depth - 1, tmp, tt, -beta, -alpha,
                                  ply + 1);
    } else {
      const bool reduce =
          allow_lmr && i >= 3 && !is_capture(board, moves[i]) &&
          moves[i].type() != PROMOTION;
      score = -alpha_beta_pruning(
          depth - 1 - static_cast<int>(reduce), tmp, tt, -alpha - 1, -alpha,
          ply + 1);
      if (reduce &&
          !g_search_state.should_stop.load(std::memory_order_relaxed) &&
          score > alpha) {
        score = -alpha_beta_pruning(depth - 1, tmp, tt, -alpha - 1, -alpha,
                                    ply + 1);
      }
      if (!g_search_state.should_stop.load(std::memory_order_relaxed) &&
          score > alpha && score < beta) {
        score = -alpha_beta_pruning(depth - 1, tmp, tt, -beta, -alpha,
                                    ply + 1);
      }
    }

    if (g_search_state.should_stop.load(std::memory_order_relaxed)) return 0;

    if (score > best) {
      best = score;
      best_move = moves[i];

      if (ply == 0)
        g_search_stats.best_move.store(best_move, std::memory_order_relaxed);
      if (score > alpha) {
        alpha = score;
        if (score >= beta) {
          record_quiet_cutoff(board, moves[i], depth, ply);
          break;
        }
      }
    }
  }

  if (g_search_state.should_stop.load(std::memory_order_relaxed)) return 0;

  const TTFlag flag = best <= original_alpha ? TT_ALPHA
                      : best >= beta          ? TT_BETA
                                              : TT_EXACT;
  tt.store(hash_key, score_to_tt(best, ply), best_move,
           static_cast<uint8_t>(depth), flag);
  return best;
}

int quiescence_search(Board& board, [[maybe_unused]] TranspositionTable& tt,
                      int alpha, int beta, int ply) {
  return quiescence_impl(board, alpha, beta, ply);
}

}  // namespace FoChess
