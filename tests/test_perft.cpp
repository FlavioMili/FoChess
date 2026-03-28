// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#include <gtest/gtest.h>

#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "board.h"
#include "fen.h"
#include "magic.h"
#include "movegen.h"
#include "zobrist.h"

uint64_t perft(Board& board, int depth) {
  if (depth == 0) return 1;

  std::array<Move, MAX_MOVES> moves;
  size_t n = MoveGen::generate_all(board, moves);

  if (depth == 1) {
    return n;
  }

  uint64_t nodes = 0;
  for (size_t i = 0; i < n; ++i) {
    Board copy = board;
    copy.makeMove(moves[i]);
    nodes += perft(copy, depth - 1);
  }

  return nodes;
}

class PerftTest : public ::testing::Test {
 protected:
  void SetUp() override {
    Bitboards::init_magic_tables();
    Zobrist::init_zobrist_keys();
  }
};

TEST_F(PerftTest, SuiteFromFile) {
  const std::string path = "resources/perft_test_list.txt";
  std::ifstream file(path);
  ASSERT_TRUE(file.is_open())
      << "Error opening file: " << path
      << "\nCurrent path: " << std::filesystem::current_path()
      << "\nFile exists: " << std::filesystem::exists(path);

  std::string line;
  while (std::getline(file, line)) {
    if (line.empty()) continue;

    std::stringstream ss(line);
    std::string fen;
    std::getline(ss, fen, ';');

    Board board = FEN::parse(fen);

    std::string segment;
    while (std::getline(ss, segment, ';')) {
      std::stringstream segment_ss(segment);
      std::string token;
      segment_ss >> token;  // e.g., "D1"
      if (token.empty() || token[0] != 'D') continue;

      int depth = std::stoi(token.substr(1));

      uint64_t expected_nodes;
      ASSERT_TRUE(segment_ss >> expected_nodes)
          << "Could not parse expected nodes for token " << token
          << " in line: " << line;

      uint64_t result = perft(board, depth);
      EXPECT_EQ(result, expected_nodes)
          << "Failed for FEN: " << fen << " at depth " << depth;
    }
  }
}
