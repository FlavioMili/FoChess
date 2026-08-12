#include <gtest/gtest.h>

#include <array>

#include "fen.h"
#include "magic.h"
#include "movegen.h"
#include "search.h"
#include "tt.h"
#include "zobrist.h"

class SearchRegressionTest : public ::testing::Test {
 protected:
  static void SetUpTestSuite() {
    Zobrist::init_zobrist_keys();
    Bitboards::init_magic_tables();
  }
};

TEST_F(SearchRegressionTest, FindsMateInOneAtDepthOne) {
  Board board = FEN::parse("7k/5Q2/6K1/8/8/8/8/8 w - - 0 1");
  TranspositionTable tt;

  FoChess::iterative_deepening(1, board, tt);
  const Move best = FoChess::g_search_stats.best_move.load();

  ASSERT_NE(best.raw(), EMPTY_MOVE);
  EXPECT_GE(FoChess::g_search_stats.best_root_score.load(),
            FoChess::MATE_SCORE - 1);

  board.makeMove(best);
  std::array<Move, MAX_MOVES> replies;
  EXPECT_EQ(MoveGen::generate_all(board, replies), 0);
  EXPECT_TRUE(board.is_in_check(board.sideToMove));
}

TEST_F(SearchRegressionTest, QuiescenceSearchesQuietCheckEvasions) {
  Board board = FEN::parse("7k/8/8/8/8/8/7R/K7 b - - 0 1");
  ASSERT_TRUE(board.is_in_check(board.sideToMove));

  FoChess::reset_search();
  const int score = FoChess::quiescence_search(
      board, -FoChess::INF_SCORE, FoChess::INF_SCORE);
  FoChess::end_search();

  EXPECT_GT(FoChess::g_search_stats.node_count.load(), 1);
  EXPECT_GT(score, -FoChess::MATE_SCORE + 128);
}

TEST_F(SearchRegressionTest, QuiescenceSearchesQuietPromotions) {
  Board board = FEN::parse("7k/P7/8/8/8/8/8/7K w - - 0 1");

  FoChess::reset_search();
  const int score = FoChess::quiescence_search(
      board, -FoChess::INF_SCORE, FoChess::INF_SCORE);
  FoChess::end_search();

  EXPECT_GT(score, 1000);
}

TEST_F(SearchRegressionTest, AbortedSearchDoesNotStoreRootEntry) {
  Board board = FEN::parse();
  TranspositionTable tt;

  FoChess::reset_search();
  FoChess::g_search_state.should_stop.store(true, std::memory_order_relaxed);
  FoChess::alpha_beta_pruning(5, board, tt);
  FoChess::end_search();

  EXPECT_EQ(tt.probe(board.hash), nullptr);
}

TEST_F(SearchRegressionTest, RecognizesSupportedRuleDraws) {
  TranspositionTable tt;

  Board fifty_move = FEN::parse("7k/8/8/8/8/8/8/R6K w - - 100 51");
  EXPECT_EQ(FoChess::alpha_beta_pruning(
                3, fifty_move, tt, -FoChess::INF_SCORE, FoChess::INF_SCORE),
            0);

  Board bare_minor = FEN::parse("7k/8/8/8/8/8/8/B6K w - - 0 1");
  EXPECT_EQ(FoChess::alpha_beta_pruning(
                3, bare_minor, tt, -FoChess::INF_SCORE, FoChess::INF_SCORE),
            0);
}

TEST_F(SearchRegressionTest, RecognizesThreefoldRepetitionFromGameHistory) {
  Board board = FEN::parse("7k/8/8/8/8/8/Q7/7K w - - 8 5");
  const uint64_t other_position = board.hash ^ 1ULL;
  TranspositionTable tt;

  const std::array<uint64_t, 5> threefold_history = {
      board.hash, other_position, board.hash, other_position, board.hash};
  FoChess::iterative_deepening(1, board, tt, true, threefold_history);
  EXPECT_EQ(FoChess::g_search_stats.best_root_score.load(), 0);

  const std::array<uint64_t, 3> twofold_history = {
      board.hash, other_position, board.hash};
  tt.clear();
  FoChess::iterative_deepening(1, board, tt, true, twofold_history);
  EXPECT_GT(FoChess::g_search_stats.best_root_score.load(), 0);
}

TEST_F(SearchRegressionTest, RecognizesStalemateAtDepthZero) {
  Board board = FEN::parse("7k/5K2/6Q1/8/8/8/8/8 b - - 0 1");
  ASSERT_FALSE(board.is_in_check(board.sideToMove));
  ASSERT_FALSE(MoveGen::has_legal_move(board));

  FoChess::reset_search();
  EXPECT_EQ(FoChess::quiescence_search(
                board, -FoChess::INF_SCORE, FoChess::INF_SCORE),
            0);
  FoChess::end_search();
}

TEST_F(SearchRegressionTest, StaticExchangeEvaluationClassifiesCaptures) {
  const Board wins_queen =
      FEN::parse("q3k3/8/8/8/8/8/8/R3K3 w - - 0 1");
  EXPECT_GT(FoChess::static_exchange_evaluation(
                wins_queen, Move(A1, A8)),
            0);

  const Board loses_queen =
      FEN::parse("4k3/8/4p3/3p4/8/8/8/3QK3 w - - 0 1");
  EXPECT_LT(FoChess::static_exchange_evaluation(
                loses_queen, Move(D1, D5)),
            0);

  const Board equal_pawns =
      FEN::parse("4k3/8/2p5/3p4/4P3/8/8/4K3 w - - 0 1");
  EXPECT_EQ(FoChess::static_exchange_evaluation(
                equal_pawns, Move(E4, D5)),
            0);
}

TEST_F(SearchRegressionTest, TaperedEvaluationUsesGamePhaseAndPawnStructure) {
  const Board start = FEN::parse();
  EXPECT_EQ(FoChess::bland_evaluate(start), 0);

  const Board central_endgame_king =
      FEN::parse("7k/8/8/8/4K3/8/8/8 w - - 0 1");
  const Board corner_endgame_king =
      FEN::parse("7k/8/8/8/8/8/8/K7 w - - 0 1");
  EXPECT_GT(FoChess::bland_evaluate(central_endgame_king),
            FoChess::bland_evaluate(corner_endgame_king));

  const Board advanced_passer =
      FEN::parse("7k/p7/8/4P3/8/8/8/7K w - - 0 1");
  const Board blocked_pawn =
      FEN::parse("7k/4p3/8/4P3/8/8/8/7K w - - 0 1");
  EXPECT_GT(FoChess::bland_evaluate(advanced_passer),
            FoChess::bland_evaluate(blocked_pawn));
}

TEST_F(SearchRegressionTest, EvaluationRewardsKingShelter) {
  const Board sheltered =
      FEN::parse("6k1/5ppp/8/8/8/8/5PPP/6K1 w - - 0 1");
  const Board exposed =
      FEN::parse("6k1/5ppp/8/8/5PPP/8/8/6K1 w - - 0 1");

  EXPECT_GT(FoChess::bland_evaluate(sheltered),
            FoChess::bland_evaluate(exposed));
}

TEST_F(SearchRegressionTest, TranspositionTablePreservesExactScores) {
  const std::array<const char*, 3> positions = {
      "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
      "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
      "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1"};

  for (const char* fen : positions) {
    Board board = FEN::parse(fen);
    FoChess::reset_search();
    const int without_tt = FoChess::alpha_beta_pruning(
        4, board, -FoChess::INF_SCORE, FoChess::INF_SCORE);
    FoChess::end_search();

    TranspositionTable tt;
    FoChess::reset_search();
    const int with_tt = FoChess::alpha_beta_pruning(
        4, board, tt, -FoChess::INF_SCORE, FoChess::INF_SCORE);
    FoChess::end_search();

    EXPECT_EQ(with_tt, without_tt) << fen;
  }
}

TEST_F(SearchRegressionTest, QuiescenceReusesExactTransposition) {
  Board board = FEN::parse(
      "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
  TranspositionTable tt;

  FoChess::reset_search();
  const int first_score = FoChess::quiescence_search(
      board, tt, -FoChess::INF_SCORE, FoChess::INF_SCORE);
  const uint64_t first_nodes = FoChess::g_search_stats.node_count.load();
  FoChess::end_search();
  ASSERT_GT(first_nodes, 0);

  FoChess::reset_search();
  const int second_score = FoChess::quiescence_search(
      board, tt, -FoChess::INF_SCORE, FoChess::INF_SCORE);
  const uint64_t second_nodes = FoChess::g_search_stats.node_count.load();
  FoChess::end_search();

  EXPECT_EQ(second_score, first_score);
  EXPECT_EQ(second_nodes, 0);
}

TEST_F(SearchRegressionTest, SelectiveSearchPreservesBenchmarkResults) {
  struct Case {
    const char* fen;
    Move expected_move;
    int expected_score;
  };
  const std::array<Case, 6> cases = {{
      {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
       Move(B1, C3), 19},
      {"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
       Move(E2, A6), 99},
      {"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", Move(B4, F4),
       108},
      {"r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
       Move(C4, C5), -949},
      {"rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
       Move(D7, C8, QUEEN), 1026},
      {"r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
       Move(C3, D5), 86},
  }};

  for (const Case& test : cases) {
    Board board = FEN::parse(test.fen);
    TranspositionTable tt;
    FoChess::iterative_deepening(5, board, tt);
    EXPECT_EQ(FoChess::g_search_stats.best_move.load(), test.expected_move)
        << test.fen;
    EXPECT_EQ(FoChess::g_search_stats.best_root_score.load(),
              test.expected_score)
        << test.fen;
  }
}
