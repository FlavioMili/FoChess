// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#pragma once
#include <array>

#include "bitboard.h"

namespace Bitboards {

constexpr Bitboard rook_mask(Square sq) noexcept {
  Bitboard result = 0;
  int r, f;
  int R = sq / 8;
  int F = sq % 8;
  for (r = R + 1; r <= 6; r++) result |= (1ULL << (r * 8 + F));
  for (r = R - 1; r >= 1; r--) result |= (1ULL << (r * 8 + F));
  for (f = F + 1; f <= 6; f++) result |= (1ULL << (R * 8 + f));
  for (f = F - 1; f >= 1; f--) result |= (1ULL << (R * 8 + f));
  return result;
}

constexpr Bitboard bishop_mask(Square sq) noexcept {
  Bitboard result = 0;
  int r, f;
  int R = sq / 8;
  int F = sq % 8;
  for (r = R + 1, f = F + 1; r <= 6 && f <= 6; r++, f++) result |= (1ULL << (r * 8 + f));
  for (r = R + 1, f = F - 1; r <= 6 && f >= 1; r++, f--) result |= (1ULL << (r * 8 + f));
  for (r = R - 1, f = F + 1; r >= 1 && f <= 6; r--, f++) result |= (1ULL << (r * 8 + f));
  for (r = R - 1, f = F - 1; r >= 1 && f >= 1; r--, f--) result |= (1ULL << (r * 8 + f));
  return result;
}

constexpr Bitboard generate_rook_attacks(Square sq, Bitboard occ) noexcept {
  Bitboard attacks = 0ULL;
  const Bitboard bb = square_bb(sq);

  for (Bitboard t = bb_up(bb); t; t = bb_up(t)) {
    attacks |= t;
    if (t & occ) break;
  }
  for (Bitboard t = bb_down(bb); t; t = bb_down(t)) {
    attacks |= t;
    if (t & occ) break;
  }
  for (Bitboard t = bb_left(bb); t; t = bb_left(t)) {
    attacks |= t;
    if (t & occ) break;
  }
  for (Bitboard t = bb_right(bb); t; t = bb_right(t)) {
    attacks |= t;
    if (t & occ) break;
  }

  return attacks;
}

constexpr Bitboard generate_bishop_attacks(Square sq, Bitboard occ) noexcept {
  Bitboard attacks = 0ULL;
  const Bitboard bb = square_bb(sq);

  for (Bitboard t = bb_up_left(bb); t; t = bb_up_left(t)) {
    attacks |= t;
    if (t & occ) break;
  }
  for (Bitboard t = bb_up_right(bb); t; t = bb_up_right(t)) {
    attacks |= t;
    if (t & occ) break;
  }
  for (Bitboard t = bb_down_left(bb); t; t = bb_down_left(t)) {
    attacks |= t;
    if (t & occ) break;
  }
  for (Bitboard t = bb_down_right(bb); t; t = bb_down_right(t)) {
    attacks |= t;
    if (t & occ) break;
  }

  return attacks;
}

// ------------------------------------------------------------
// Occupancy builder
// ------------------------------------------------------------

constexpr Bitboard set_occupancy_from_index(size_t index, Bitboard mask) noexcept {
  Bitboard occ = 0ULL;
  for (int sq = 0; sq < 64; ++sq) {
    if (mask & (1ULL << sq)) {
      if (index & 1ULL) occ |= 1ULL << sq;
      index >>= 1;
    }
  }
  return occ;
}

struct SMagic {
  Bitboard mask;
  Bitboard magic;
  uint8_t shift;
};

constexpr std::array<SMagic, 64> rook_magics = {
    SMagic{0x101010101017eULL, 0x1080004002518820ULL, 52},     // Rook 0
    SMagic{0x202020202027cULL, 0x1180108008200140ULL, 52},     // Rook 1
    SMagic{0x404040404047aULL, 0x200802a000140018ULL, 52},     // Rook 2
    SMagic{0x8080808080876ULL, 0x80100100080280eULL, 52},      // Rook 3
    SMagic{0x1010101010106eULL, 0x420020020400100ULL, 52},     // Rook 4
    SMagic{0x2020202020205eULL, 0x12000c020002c801ULL, 52},    // Rook 5
    SMagic{0x4040404040403eULL, 0x2200260008008041ULL, 52},    // Rook 6
    SMagic{0x8080808080807eULL, 0x100005204802900ULL, 52},     // Rook 7
    SMagic{0x1010101017e00ULL, 0x50200900210010e4ULL, 52},     // Rook 8
    SMagic{0x2020202027c00ULL, 0x46002400842ULL, 52},          // Rook 9
    SMagic{0x4040404047a00ULL, 0x4804801242600008ULL, 52},     // Rook 10
    SMagic{0x8080808087600ULL, 0x60a400212042800ULL, 52},      // Rook 11
    SMagic{0x10101010106e00ULL, 0xc014208801000800ULL, 52},    // Rook 12
    SMagic{0x20202020205e00ULL, 0x101202001000180ULL, 52},     // Rook 13
    SMagic{0x40404040403e00ULL, 0x8800822028090407ULL, 52},    // Rook 14
    SMagic{0x80808080807e00ULL, 0x9601061010140ULL, 52},       // Rook 15
    SMagic{0x10101017e0100ULL, 0x208000400010ULL, 52},         // Rook 16
    SMagic{0x20202027c0200ULL, 0xc002200410402000ULL, 52},     // Rook 17
    SMagic{0x40404047a0400ULL, 0x2848002010000400ULL, 52},     // Rook 18
    SMagic{0x8080808760800ULL, 0x2184009000200200ULL, 52},     // Rook 19
    SMagic{0x101010106e1000ULL, 0x842e100101c0608ULL, 52},     // Rook 20
    SMagic{0x202020205e2000ULL, 0x10480404011010ULL, 52},      // Rook 21
    SMagic{0x404040403e4000ULL, 0x230c01080a050ULL, 52},       // Rook 22
    SMagic{0x808080807e8000ULL, 0x2809c0088024100ULL, 52},     // Rook 23
    SMagic{0x101017e010100ULL, 0x8000a120009000ULL, 52},       // Rook 24
    SMagic{0x202027c020200ULL, 0x190880220004020ULL, 52},      // Rook 25
    SMagic{0x404047a040400ULL, 0x100544102000e980ULL, 52},     // Rook 26
    SMagic{0x8080876080800ULL, 0x802001040200ULL, 52},         // Rook 27
    SMagic{0x1010106e101000ULL, 0x10500210410030ULL, 52},      // Rook 28
    SMagic{0x2020205e202000ULL, 0x1040800410030400ULL, 52},    // Rook 29
    SMagic{0x4040403e404000ULL, 0xa0000804024010a0ULL, 52},    // Rook 30
    SMagic{0x8080807e808000ULL, 0x10412010800c2041ULL, 52},    // Rook 31
    SMagic{0x1017e01010100ULL, 0x8010029510900100ULL, 52},     // Rook 32
    SMagic{0x2027c02020200ULL, 0x200200280440050ULL, 52},      // Rook 33
    SMagic{0x4047a04040400ULL, 0x30000800200420ULL, 52},       // Rook 34
    SMagic{0x8087608080800ULL, 0x805100920201d1ULL, 52},       // Rook 35
    SMagic{0x10106e10101000ULL, 0x300280400320040ULL, 52},     // Rook 36
    SMagic{0x20205e20202000ULL, 0x60000882005c0004ULL, 52},    // Rook 37
    SMagic{0x40403e40404000ULL, 0x200800040201100ULL, 52},     // Rook 38
    SMagic{0x80807e80808000ULL, 0x100002b8230ULL, 52},         // Rook 39
    SMagic{0x17e0101010100ULL, 0x6008010400c8000ULL, 52},      // Rook 40
    SMagic{0x27c0202020200ULL, 0x41000a15c0001014ULL, 52},     // Rook 41
    SMagic{0x47a0404040400ULL, 0x20004030210008ULL, 52},       // Rook 42
    SMagic{0x8760808080800ULL, 0x21000200101000ULL, 52},       // Rook 43
    SMagic{0x106e1010101000ULL, 0x4441002330010098ULL, 52},    // Rook 44
    SMagic{0x205e2020202000ULL, 0x110c186020001ULL, 52},       // Rook 45
    SMagic{0x403e4040404000ULL, 0x8000106200008001ULL, 52},    // Rook 46
    SMagic{0x807e8080808000ULL, 0x4031020004ULL, 52},          // Rook 47
    SMagic{0x7e010101010100ULL, 0x2500442888008208ULL, 52},    // Rook 48
    SMagic{0x7c020202020200ULL, 0x104880180010020cULL, 52},    // Rook 49
    SMagic{0x7a040404040400ULL, 0x814000805000428ULL, 52},     // Rook 50
    SMagic{0x76080808080800ULL, 0x800020480400820ULL, 52},     // Rook 51
    SMagic{0x6e101010101000ULL, 0x9108000100081100ULL, 52},    // Rook 52
    SMagic{0x5e202020202000ULL, 0x802204400120a08ULL, 52},     // Rook 53
    SMagic{0x3e404040404000ULL, 0x880080090c12200ULL, 52},     // Rook 54
    SMagic{0x7e808080808000ULL, 0xc001810000254018ULL, 52},    // Rook 55
    SMagic{0x7e01010101010100ULL, 0x1050ae050408001ULL, 52},   // Rook 56
    SMagic{0x7c02020202020200ULL, 0x403220810804001ULL, 52},   // Rook 57
    SMagic{0x7a04040404040400ULL, 0x2081c01204a20082ULL, 52},  // Rook 58
    SMagic{0x7608080808080800ULL, 0x840440082002000eULL, 52},  // Rook 59
    SMagic{0x6e10101010101000ULL, 0x4001020210080001ULL, 52},  // Rook 60
    SMagic{0x5e20202020202000ULL, 0x46000414008102ULL, 52},    // Rook 61
    SMagic{0x3e40404040404000ULL, 0x801000200118051ULL, 52},   // Rook 62
    SMagic{0x7e80808080808000ULL, 0x100000810c104022ULL, 52},  // Rook 63
};

constexpr std::array<SMagic, 64> bishop_magics = {
    SMagic{0x40201008040200ULL, 0x4a0c003218401200ULL, 55},  // Bishop 0
    SMagic{0x402010080400ULL, 0x81180208400800ULL, 55},      // Bishop 1
    SMagic{0x4020100a00ULL, 0x1940a080258a0050ULL, 55},      // Bishop 2
    SMagic{0x40221400ULL, 0x380d22002081000ULL, 55},         // Bishop 3
    SMagic{0x2442800ULL, 0x4042000000000ULL, 55},            // Bishop 4
    SMagic{0x204085000ULL, 0x105092020008ULL, 55},           // Bishop 5
    SMagic{0x20408102000ULL, 0x210822101808ULL, 55},         // Bishop 6
    SMagic{0x2040810204000ULL, 0x424064004000ULL, 55},       // Bishop 7
    SMagic{0x20100804020000ULL, 0x300200c00460028ULL, 55},   // Bishop 8
    SMagic{0x40201008040000ULL, 0x50501122401018ULL, 55},    // Bishop 9
    SMagic{0x4020100a0000ULL, 0x1001381800400900ULL, 55},    // Bishop 10
    SMagic{0x4022140000ULL, 0x200108080100080ULL, 55},       // Bishop 11
    SMagic{0x244280000ULL, 0x40023000140ULL, 55},            // Bishop 12
    SMagic{0x20408500000ULL, 0x340069808820050ULL, 55},      // Bishop 13
    SMagic{0x2040810200000ULL, 0x4001000429080800ULL, 55},   // Bishop 14
    SMagic{0x4081020400000ULL, 0x8004204440221ULL, 55},      // Bishop 15
    SMagic{0x10080402000200ULL, 0x800840a02100400ULL, 55},   // Bishop 16
    SMagic{0x20100804000400ULL, 0x812042c104042034ULL, 55},  // Bishop 17
    SMagic{0x4020100a000a00ULL, 0x440102604040c0ULL, 55},    // Bishop 18
    SMagic{0x402214001400ULL, 0x202004904440800ULL, 55},     // Bishop 19
    SMagic{0x24428002800ULL, 0x8004900a02008080ULL, 55},     // Bishop 20
    SMagic{0x2040850005000ULL, 0x8000804010002002ULL, 55},   // Bishop 21
    SMagic{0x4081020002000ULL, 0x44229040118ULL, 55},        // Bishop 22
    SMagic{0x8102040004000ULL, 0x1a000b9000811ULL, 55},      // Bishop 23
    SMagic{0x8040200020400ULL, 0x15180009201a9ULL, 55},      // Bishop 24
    SMagic{0x10080400040800ULL, 0x400010102402a080ULL, 55},  // Bishop 25
    SMagic{0x20100a000a1000ULL, 0x808088040068000cULL, 55},  // Bishop 26
    SMagic{0x40221400142200ULL, 0x40802002020200ULL, 55},    // Bishop 27
    SMagic{0x2442800284400ULL, 0x116840000802000ULL, 55},    // Bishop 28
    SMagic{0x4085000500800ULL, 0x22010026c1120128ULL, 55},   // Bishop 29
    SMagic{0x8102000201000ULL, 0x8208240101049120ULL, 55},   // Bishop 30
    SMagic{0x10204000402000ULL, 0x2e095001010800ULL, 55},    // Bishop 31
    SMagic{0x4020002040800ULL, 0x2404600418a08050ULL, 55},   // Bishop 32
    SMagic{0x8040004081000ULL, 0xe108030042010820ULL, 55},   // Bishop 33
    SMagic{0x100a000a102000ULL, 0x2002082884030011ULL, 55},  // Bishop 34
    SMagic{0x22140014224000ULL, 0x6020083080081ULL, 55},     // Bishop 35
    SMagic{0x44280028440200ULL, 0x200404040040100ULL, 55},   // Bishop 36
    SMagic{0x8500050080400ULL, 0x1031802200010080ULL, 55},   // Bishop 37
    SMagic{0x10200020100800ULL, 0x8f79030600000450ULL, 55},  // Bishop 38
    SMagic{0x20400040201000ULL, 0x4249000408082304ULL, 55},  // Bishop 39
    SMagic{0x2000204081000ULL, 0x4801041a2001702ULL, 55},    // Bishop 40
    SMagic{0x4000408102000ULL, 0x300101006000100ULL, 55},    // Bishop 41
    SMagic{0xa000a10204000ULL, 0x1002000901202844ULL, 55},   // Bishop 42
    SMagic{0x14001422400000ULL, 0x340012818000020ULL, 55},   // Bishop 43
    SMagic{0x28002844020000ULL, 0x804420224021040ULL, 55},   // Bishop 44
    SMagic{0x50005008040200ULL, 0x205600c0300a008ULL, 55},   // Bishop 45
    SMagic{0x20002010080400ULL, 0x801881204002080ULL, 55},   // Bishop 46
    SMagic{0x40004020100800ULL, 0xa2840a1082100429ULL, 55},  // Bishop 47
    SMagic{0x20408102000ULL, 0x3401004805000ULL, 55},        // Bishop 48
    SMagic{0x40810204000ULL, 0x1010100e608018ULL, 55},       // Bishop 49
    SMagic{0xa1020400000ULL, 0x26008828020802ULL, 55},       // Bishop 50
    SMagic{0x142240000000ULL, 0x8404004002114000ULL, 55},    // Bishop 51
    SMagic{0x284402000000ULL, 0x20002005004c046ULL, 55},     // Bishop 52
    SMagic{0x500804020000ULL, 0x10010200020c080ULL, 55},     // Bishop 53
    SMagic{0x201008040200ULL, 0x108840015401000ULL, 55},     // Bishop 54
    SMagic{0x402010080400ULL, 0x8002030020690118ULL, 55},    // Bishop 55
    SMagic{0x2040810204000ULL, 0x1c01440900210408ULL, 55},   // Bishop 56
    SMagic{0x4081020400000ULL, 0x10c010028208310ULL, 55},    // Bishop 57
    SMagic{0xa102040000000ULL, 0x8080c0d1082a00ULL, 55},     // Bishop 58
    SMagic{0x14224000000000ULL, 0xa780031041010441ULL, 55},  // Bishop 59
    SMagic{0x28440200000000ULL, 0x42401002090a4080ULL, 55},  // Bishop 60
    SMagic{0x50080402000000ULL, 0x420006400414088ULL, 55},   // Bishop 61
    SMagic{0x20100804020000ULL, 0x400000c800c80080ULL, 55},  // Bishop 62
    SMagic{0x40201008040200ULL, 0x204820c008600ULL, 55},     // Bishop 63
};

inline std::array<std::array<Bitboard, 4096>, 64> rook_attack_table;
inline std::array<std::array<Bitboard, 512>, 64> bishop_attack_table;

inline void init_magic_tables() {
  for (size_t sq = 0; sq < 64; ++sq) {
    const SMagic& m = rook_magics[sq];
    int relevant_bits = 64 - m.shift;

    for (size_t i = 0; i < (1ULL << relevant_bits); ++i) {
      Bitboard occ = set_occupancy_from_index(i, m.mask);
      Bitboard attacks = generate_rook_attacks(Square(sq), occ);
      auto idx = ((occ & m.mask) * m.magic) >> m.shift;
      rook_attack_table[sq][idx] = attacks;
    }
  }

  // Initialize bishop attack table
  for (size_t sq = 0; sq < 64; ++sq) {
    const SMagic& m = bishop_magics[sq];
    int relevant_bits = 64 - m.shift;

    for (size_t i = 0; i < (1ULL << relevant_bits); ++i) {
      Bitboard occ = set_occupancy_from_index(i, m.mask);
      Bitboard attacks = generate_bishop_attacks(Square(sq), occ);
      auto idx = ((occ & m.mask) * m.magic) >> m.shift;
      bishop_attack_table[sq][idx] = attacks;
    }
  }
}

// ------------------------------------------------------------
///  lookup functions
// ------------------------------------------------------------
constexpr Bitboard rook_attacks(Square sq, Bitboard occ) {
  const SMagic& m = rook_magics[sq];
  auto idx = ((occ & m.mask) * m.magic) >> m.shift;
  return rook_attack_table[sq][idx];
}

constexpr Bitboard bishop_attacks(Square sq, Bitboard occ) {
  const SMagic& m = bishop_magics[sq];
  auto idx = ((occ & m.mask) * m.magic) >> m.shift;
  return bishop_attack_table[sq][idx];
}

constexpr Bitboard queen_attacks(Square sq, Bitboard occ) {
  return rook_attacks(sq, occ) | bishop_attacks(sq, occ);
}

constexpr Bitboard rook_moves(Square sq, Bitboard occ, Bitboard friendly) {
  return rook_attacks(sq, occ) & ~friendly;
}

constexpr Bitboard bishop_moves(Square sq, Bitboard occ, Bitboard friendly) {
  return bishop_attacks(sq, occ) & ~friendly;
}

constexpr Bitboard queen_moves(Square sq, Bitboard occ, Bitboard friendly) {
  return queen_attacks(sq, occ) & ~friendly;
}

}  // namespace Bitboards
