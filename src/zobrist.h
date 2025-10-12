#pragma once

#include <array>
#include <cstddef>

#include "bitboard.h"
#include "board.h"

constexpr int SEED = 0xC0FFEE;

namespace Zobrist {

// this is Java's random number algorithm
// I use this and not STL because of compile time availability
constexpr uint64_t splitmix64(uint64_t& state) {
  uint64_t z = (state += 0x9E3779B97F4A7C15ULL);
  z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
  z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
  return z ^ (z >> 31);
}

constexpr auto init_pieces() {
  std::array<std::array<std::array<uint64_t, 64>, 6>, 2> pieces{};

  uint64_t state = SEED;
  for (size_t c = 0; c < 2; ++c)
    for (size_t p = 0; p < 6; ++p)
      for (size_t sq = 0; sq < 64; ++sq) pieces[c][p][sq] = splitmix64(state);

  return pieces;
}

constexpr auto init_en_passant() {
  std::array<uint64_t, 64> keys{};
  uint64_t state = SEED + 1;
  for (auto& k : keys) k = splitmix64(state);
  return keys;
}

constexpr auto init_castling() {
  std::array<uint64_t, 16> keys{};
  uint64_t state = SEED + 2;
  for (auto& k : keys) k = splitmix64(state);
  return keys;
}

constexpr uint64_t init_side_to_move() {
  uint64_t state = SEED + 3;
  return splitmix64(state);
}

constexpr void init_zobrist_keys() {
  init_pieces();
  init_en_passant();
  init_castling();
  init_side_to_move();
}

constexpr std::array<std::array<std::array<uint64_t, 64>, 6>, 2> pieces_keys = init_pieces();
constexpr std::array<Bitboard, 64> enPassant_keys = init_en_passant();
constexpr std::array<Bitboard, 16> castling_keys = init_castling();
constexpr Bitboard sideToMove_key = init_side_to_move();


Bitboard generate_hash(const Board& board);


}  // namespace Zobrist
