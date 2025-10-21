#include <array>
#include <chrono>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "board.h"
#include "fen.h"
#include "magic.h"
#include "movegen.h"

uint64_t perft(Board& board, int depth) {
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

int main() {
  Bitboards::init_magic_tables();

  std::ifstream file("resources/perft_test_list.txt");
  if (!file.is_open()) {
    std::cerr << "Error opening file: ../resources/perft_test_list.txt"
              << std::endl;
    return 1;
  }

  std::vector<std::string> lines;
  std::string line;
  while (std::getline(file, line)) {
    if (!line.empty()) {
      lines.push_back(line);
    }
  }
  file.close();

  int total_lines = lines.size();
  int lines_processed = 0;

  auto total_start_time = std::chrono::high_resolution_clock::now();

  for (const auto& current_line : lines) {
    std::stringstream ss(current_line);
    std::string fen;
    std::getline(ss, fen, ';');

    Board board = FEN::parse(fen);

    std::string segment;
    while (std::getline(ss, segment, ';')) {
      std::stringstream segment_ss(segment);
      std::string token;
      segment_ss >> token;  // e.g., "D1"
      if (token.empty() || token[0] != 'D') {
        continue;
      }

      int depth = 0;
      depth = std::stoi(token.substr(1));

      uint64_t expected_nodes;
      if (!(segment_ss >> expected_nodes)) {
        std::cerr << "Could not parse expected nodes for token " << token
                  << " in line: " << current_line << std::endl;
        continue;
      }

      uint64_t result = perft(board, depth);

      if (result != expected_nodes) {
        std::cerr << "\n--- TEST FAILED ---\n";
        std::cerr << "FEN: " << fen << "\n";
        std::cerr << "Depth: " << depth << "\n"; 
        std::cerr << "given: " << result << ", expected: " << expected_nodes << "\n";
        std::cerr << "-------------------\n";
        return 1;
      }
    }

    lines_processed++;
    int percentage = (int)(((double)lines_processed / total_lines) * 100.0);
    std::cout << "[" << percentage << "% processed]\n";
  }

  auto total_end_time = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
      total_end_time - total_start_time);

  std::cout << "\nAll perft tests passed in " << duration.count() << "ms!\n";

  return 0;
}
