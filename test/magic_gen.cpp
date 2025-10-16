// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------



// this file is a mess but seems like it gave ok results

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <random>

#include "bitboard.h"
#include "magic.h"

using namespace Bitboards;

/*********************************************************************
                      MAGIC BITBOARD GENERATION
*********************************************************************/

inline bool generate_magics_with_seed() {
  std::random_device rd;
  std::mt19937_64 gen(rd());
  auto random_sparse_64 = [&]() { return gen() & gen() & gen(); };

  auto find_magic_seeded = [&](size_t sq, int relevant_bits, bool bishop, SMagic& out) {
    const size_t table_size = 1u << relevant_bits;
    std::vector<Bitboard> used(table_size, 0ULL);

    Bitboard mask = bishop ? bishop_mask(static_cast<Square>(sq))
                           : rook_mask(static_cast<Square>(sq));
    uint8_t shift = static_cast<uint8_t>(64 - relevant_bits);

    for (int i = 0; i < 100000000; ++i) {
      Bitboard magic = random_sparse_64();
      if (Bitboards::popcount((mask * magic) & 0xFF00000000000000ULL) < 6) continue;

      std::fill(used.begin(), used.end(), 0ULL);
      bool fail = false;

      for (size_t j = 0; j < (1ULL << relevant_bits); ++j) {
        Bitboard occ = set_occupancy_from_index(j, mask);
        Bitboard attack = bishop ? generate_bishop_attacks(static_cast<Square>(sq), occ)
                                 : generate_rook_attacks(static_cast<Square>(sq), occ);
        size_t idx = static_cast<size_t>(((occ & mask) * magic) >> shift);

        if (idx >= used.size()) {
          fail = true;
          break;
        }
        if (!used[idx])
          used[idx] = attack;
        else if (used[idx] != attack) {
          fail = true;
          break;
        }
      }

      if (!fail) {
        out.magic = magic;
        out.mask = mask;
        out.shift = shift;
        return true;
      }
    }
    return false;
  };

  bool all_ok = true;
  std::array<SMagic, 64> rooks{}, bishops{};

  for (size_t sq = 0; sq < 64; ++sq) {
    if (!find_magic_seeded(sq, 12, false, rooks[sq])) {
      std::cerr << " failed at rook " << sq << "\n";
      all_ok = false;
      break;
    }
  }
  for (size_t sq = 0; sq < 64 && all_ok; ++sq) {
    if (!find_magic_seeded(sq, 9, true, bishops[sq])) {
      std::cerr << " failed at bishop " << sq << "\n";
      all_ok = false;
      break;
    }
  }

  if (all_ok) {
    std::cout << "succeeded.\n";
    std::cout << "constexpr std::array<SMagic,64> rook_magics = {\n";
    for (size_t sq = 0; sq < 64; ++sq)
      std::cout << "  SMagic{0x" << std::hex << rooks[sq].mask << "ULL, 0x" << rooks[sq].magic
                << "ULL, " << std::dec << rooks[sq].shift << "}, // Rook " << sq << "\n";
    std::cout << "};\n\nconstexpr std::array<SMagic,64> bishop_magics = {\n";
    for (size_t sq = 0; sq < 64; ++sq)
      std::cout << "  SMagic{0x" << std::hex << bishops[sq].mask << "ULL, 0x" << bishops[sq].magic
                << "ULL, " << std::dec << bishops[sq].shift << "}, // Bishop " << sq << "\n";
    std::cout << "};\n";
  }

  return all_ok;
}

int main() {
  while (!generate_magics_with_seed()) {
    std::cerr << "Retrying ...\n";
  }
  return 0;
}
