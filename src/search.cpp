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
#include <vector>

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
std::array<uint64_t, MAX_SEARCH_PLY> search_path{};
std::vector<uint64_t> prior_game_history;
int null_searches = 0;

bool is_repetition(const Board& board, int ply) {
  if (null_searches != 0 || board.halfMoveClock < 4) return false;

  int matches = 0;
  for (int path_ply = ply - 2; path_ply >= 0; path_ply -= 2) {
    const int distance = ply - path_ply;
    if (distance > board.halfMoveClock) break;
    if (search_path[static_cast<size_t>(path_ply)] == board.hash &&
        ++matches == 2)
      return true;
  }

  const size_t history_size = prior_game_history.size();
  for (size_t offset = 1; offset <= history_size; ++offset) {
    const size_t distance = static_cast<size_t>(ply) + offset;
    if (distance > board.halfMoveClock) break;
    if (prior_game_history[history_size - offset] == board.hash &&
        ++matches == 2)
      return true;
  }
  return false;
}

void record_search_position(const Board& board, int ply) {
  if (ply >= 0 && ply < MAX_SEARCH_PLY)
    search_path[static_cast<size_t>(ply)] = board.hash;
}

bool is_capture(const Board& board, const Move& move) {
  return move.type() == EN_PASSANT ||
         (Bitboards::square_bb(move.to_sq()) &
          board.occupancy[BLACK - board.sideToMove]) != 0;
}

Bitboard attackers_to(Square target, Color side, Bitboard occupancy,
                      const std::array<std::array<Bitboard, 6>, 2>& pieces) {
  return (Bitboards::pawn_attacks_mask(
              target, static_cast<Color>(BLACK - side)) &
          pieces[side][PAWN]) |
         (Bitboards::knight_attacks(target) & pieces[side][KNIGHT]) |
         (Bitboards::bishop_attacks(target, occupancy) &
          (pieces[side][BISHOP] | pieces[side][QUEEN])) |
         (Bitboards::rook_attacks(target, occupancy) &
          (pieces[side][ROOK] | pieces[side][QUEEN])) |
         (Bitboards::king_attacks(target) & pieces[side][KING]);
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
    if (victim != NO_PIECE && attacker != NO_PIECE) {
      const int see = static_exchange_evaluation(board, move);
      score += (see >= 0 ? 4'000'000 : -1'000'000) + 32 * see +
               16 * ORDER_PIECE_VALUES[victim] -
               ORDER_PIECE_VALUES[attacker];
    }
    return score;
  }

  if (ply < MAX_SEARCH_PLY) {
    if (move == killer_moves[static_cast<size_t>(ply)][0]) return 3'000'000;
    if (move == killer_moves[static_cast<size_t>(ply)][1]) return 2'900'000;
  }

  return score + history_scores[board.sideToMove][move.from_sq()][move.to_sq()];
}

class MovePicker {
 public:
  MovePicker(const Board& board, std::array<Move, MAX_MOVES>& moves,
             size_t count, const Move& tt_move, int ply)
      : moves_(moves), count_(count) {
    for (size_t i = 0; i < count_; ++i) {
      scores_[i] = move_order_score(board, moves_[i], tt_move, ply);
    }
  }

  Move next() {
    size_t best = current_;
    for (size_t i = current_ + 1; i < count_; ++i) {
      if (scores_[i] > scores_[best]) best = i;
    }

    std::swap(moves_[current_], moves_[best]);
    std::swap(scores_[current_], scores_[best]);
    return moves_[current_++];
  }

 private:
  std::array<Move, MAX_MOVES>& moves_;
  std::array<int, MAX_MOVES> scores_{};
  size_t count_;
  size_t current_ = 0;
};

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

int late_move_reduction(int depth, size_t move_index) {
  int reduction = 1;
  if (depth >= 6 && move_index >= 8) ++reduction;
  if (depth >= 9 && move_index >= 16) ++reduction;
  return std::min(reduction, depth - 2);
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

  if (is_repetition(board, ply)) return 0;
  record_search_position(board, ply);

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

  MovePicker move_picker(board, moves, count, Move::none(), ply);

  for (size_t i = 0; i < count; ++i) {
    const Move move = move_picker.next();
    Board child = board;
    child.makeMove(move);

    if (!in_check && move.type() != PROMOTION && is_capture(board, move)) {
      const Piece victim = move.type() == EN_PASSANT
                               ? PAWN
                               : board.piece_on(move.to_sq());
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

int static_exchange_evaluation(const Board& board, const Move& move) {
  if (!is_capture(board, move)) return 0;

  const Color us = board.sideToMove;
  const Color them = static_cast<Color>(BLACK - us);
  const Square from = move.from_sq();
  const Square target = move.to_sq();
  const Bitboard from_bb = Bitboards::square_bb(from);
  const Bitboard target_bb = Bitboards::square_bb(target);

  const Piece moving_piece = board.piece_on(from);
  const Piece captured_piece =
      move.type() == EN_PASSANT ? PAWN : board.piece_on(target);
  if (moving_piece == NO_PIECE || captured_piece == NO_PIECE) return 0;

  auto pieces = board.pieces;
  Bitboard occupancy = board.allPieces;
  pieces[us][moving_piece] &= ~from_bb;
  occupancy &= ~from_bb;

  if (move.type() == EN_PASSANT) {
    const Square captured_square =
        us == WHITE ? Bitboards::down(target) : Bitboards::up(target);
    const Bitboard captured_bb = Bitboards::square_bb(captured_square);
    pieces[them][PAWN] &= ~captured_bb;
    occupancy &= ~captured_bb;
  } else {
    pieces[them][captured_piece] &= ~target_bb;
  }

  Piece piece_on_target = moving_piece;
  int promotion_gain = 0;
  if (move.type() == PROMOTION) {
    piece_on_target = move.promotion_type();
    promotion_gain = ORDER_PIECE_VALUES[piece_on_target] -
                     ORDER_PIECE_VALUES[PAWN];
  }
  pieces[us][piece_on_target] |= target_bb;
  occupancy |= target_bb;

  std::array<int, 32> gains{};
  gains[0] = ORDER_PIECE_VALUES[captured_piece] + promotion_gain;
  int exchange_depth = 0;
  Color side = them;

  while (exchange_depth + 1 < static_cast<int>(gains.size())) {
    const Bitboard attackers = attackers_to(target, side, occupancy, pieces);
    if (attackers == 0) break;

    Piece attacker = NO_PIECE;
    Bitboard attacker_bb = 0;
    for (int piece = PAWN; piece <= KING; ++piece) {
      attacker_bb = attackers & pieces[side][static_cast<size_t>(piece)];
      if (attacker_bb != 0) {
        attacker = static_cast<Piece>(piece);
        break;
      }
    }
    if (attacker == NO_PIECE) break;

    ++exchange_depth;
    gains[static_cast<size_t>(exchange_depth)] =
        ORDER_PIECE_VALUES[piece_on_target] -
        gains[static_cast<size_t>(exchange_depth - 1)];

    const Bitboard least_attacker = attacker_bb & (~attacker_bb + 1);
    pieces[side][attacker] &= ~least_attacker;
    occupancy &= ~least_attacker;
    piece_on_target = attacker;
    side = static_cast<Color>(BLACK - side);
  }

  while (exchange_depth > 0) {
    gains[static_cast<size_t>(exchange_depth - 1)] =
        -std::max(-gains[static_cast<size_t>(exchange_depth - 1)],
                  gains[static_cast<size_t>(exchange_depth)]);
    --exchange_depth;
  }
  return gains[0];
}

void prepare_repetition_history(uint64_t root_hash,
                                std::span<const uint64_t> history) {
  prior_game_history.assign(history.begin(), history.end());
  if (!prior_game_history.empty() && prior_game_history.back() == root_hash)
    prior_game_history.pop_back();
  search_path.fill(0);
  null_searches = 0;
}

void reset_search_heuristics() {
  for (auto& killers : killer_moves)
    killers = {Move::none(), Move::none()};
  for (auto& color : history_scores)
    for (auto& from : color) from.fill(0);
}

int alpha_beta_pruning(int depth, Board& board, int alpha, int beta, int ply,
                       bool allow_null) {
  if ((g_search_stats.node_count.load(std::memory_order_relaxed) & 2047) == 0 &&
      should_stop_search()) {
    return 0;
  }

  if (is_repetition(board, ply)) return 0;
  record_search_position(board, ply);

  if (depth == 0) return quiescence_search(board, alpha, beta, ply);

  if (rule_draw(board)) {
    if (!board.is_in_check(board.sideToMove) || board.halfMoveClock < 100)
      return 0;
    std::array<Move, MAX_MOVES> evasions;
    if (MoveGen::generate_all(board, evasions) == 0)
      return -MATE_SCORE + ply;
    return 0;
  }

  if (allow_null && depth >= 3 && beta - alpha == 1 &&
      beta > -TT_MATE_THRESHOLD && beta < TT_MATE_THRESHOLD &&
      !board.is_in_check(board.sideToMove)) {
    const Color us = board.sideToMove;
    const Bitboard non_pawn_material =
        board.pieces[us][KNIGHT] | board.pieces[us][BISHOP] |
        board.pieces[us][ROOK] | board.pieces[us][QUEEN];
    if (non_pawn_material != 0 && bland_evaluate(board) >= beta) {
      Board null_board = board;
      null_board.makeNullMove();
      const int reduction = 2 + depth / 4;
      const int null_depth = std::max(0, depth - 1 - reduction);
      ++null_searches;
      const int score = -alpha_beta_pruning(
          null_depth, null_board, -beta, -beta + 1, ply + 1, false);
      --null_searches;
      if (g_search_state.should_stop.load(std::memory_order_relaxed)) return 0;
      if (score >= beta) return beta;
    }
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

  MovePicker move_picker(board, moves, n, Move::none(), ply);

  for (size_t i = 0; i < n; i++) {
    const Move move = move_picker.next();
    Board tmp = board;
    tmp.makeMove(move);
    int score;
    if (i == 0) {
      score =
          -alpha_beta_pruning(depth - 1, tmp, -beta, -alpha, ply + 1);
    } else {
      const bool reduce =
          allow_lmr && i >= 3 && !is_capture(board, move) &&
          move.type() != PROMOTION;
      const int reduction = reduce ? late_move_reduction(depth, i) : 0;
      score = -alpha_beta_pruning(depth - 1 - reduction, tmp,
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
      best_move = move;
    }

    alpha = std::max(alpha, score);
    if (alpha >= beta) {
      record_quiet_cutoff(board, move, depth, ply);
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
                       int alpha, int beta, int ply, bool allow_null) {
  if ((g_search_stats.node_count.load(std::memory_order_relaxed) & 2047) == 0 &&
      should_stop_search())
    return 0;

  if (is_repetition(board, ply)) return 0;
  record_search_position(board, ply);

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

  if (allow_null && depth >= 3 && beta - alpha == 1 &&
      beta > -TT_MATE_THRESHOLD && beta < TT_MATE_THRESHOLD &&
      !board.is_in_check(board.sideToMove)) {
    const Color us = board.sideToMove;
    const Bitboard non_pawn_material =
        board.pieces[us][KNIGHT] | board.pieces[us][BISHOP] |
        board.pieces[us][ROOK] | board.pieces[us][QUEEN];
    if (non_pawn_material != 0 && bland_evaluate(board) >= beta) {
      Board null_board = board;
      null_board.makeNullMove();
      const int reduction = 2 + depth / 4;
      const int null_depth = std::max(0, depth - 1 - reduction);
      ++null_searches;
      const int score = -alpha_beta_pruning(
          null_depth, null_board, tt, -beta, -beta + 1, ply + 1, false);
      --null_searches;
      if (g_search_state.should_stop.load(std::memory_order_relaxed)) return 0;
      if (score >= beta) return beta;
    }
  }

  FoChess::g_search_stats.node_count.fetch_add(1, std::memory_order_relaxed);

  std::array<Move, MAX_MOVES> moves;
  size_t n = MoveGen::generate_all(board, moves);
  if (n == 0)
    return (board.is_in_check(board.sideToMove)) ? -MATE_SCORE + ply : 0;

  MovePicker move_picker(board, moves, n, tt_move, ply);

  int best = -INF_SCORE;
  Move best_move = Move::none();
  const bool allow_lmr =
      depth >= 3 && !board.is_in_check(board.sideToMove);

  const int original_alpha = alpha;

  for (size_t i = 0; i < n; ++i) {
    const Move move = move_picker.next();
    Board tmp = board;
    tmp.makeMove(move);

    int score;
    if (i == 0) {
      score = -alpha_beta_pruning(depth - 1, tmp, tt, -beta, -alpha,
                                  ply + 1);
    } else {
      const bool reduce =
          allow_lmr && i >= 3 && !is_capture(board, move) &&
          move.type() != PROMOTION;
      const int reduction = reduce ? late_move_reduction(depth, i) : 0;
      score = -alpha_beta_pruning(
          depth - 1 - reduction, tmp, tt, -alpha - 1, -alpha,
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
      best_move = move;

      if (ply == 0)
        g_search_stats.best_move.store(best_move, std::memory_order_relaxed);
      if (score > alpha) {
        alpha = score;
        if (score >= beta) {
          record_quiet_cutoff(board, move, depth, ply);
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
