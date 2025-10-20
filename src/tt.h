// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <vector>

#include "move.h"

enum TTFlag : uint8_t {
  TT_NONE = 0,
  TT_EXACT = 1,
  TT_ALPHA = 2,
  TT_BETA = 3,
};

struct TTEntry {
  uint64_t hash_key;
  int score;
  Move best_move;
  uint8_t depth;
  TTFlag flag;

  TTEntry() : hash_key(0), score(0), best_move(Move()), depth(0), flag(TT_NONE) {}
};

class TranspositionTable {
 public:
#ifdef DEBUG
  int hits = 0;
#endif
  explicit TranspositionTable(size_t mb_size = 64) {
    size_t num_entries = (mb_size * 1024 * 1024) / sizeof(TTEntry);
    // Round down to power of 2
    num_entries = 1ULL << (63 - __builtin_clzll(num_entries));
    table.resize(num_entries);
    mask = num_entries - 1;
  }

  void store(uint64_t key, int score, Move move, uint8_t depth, TTFlag flag) {
    size_t index = key & mask;
    TTEntry& entry = table[index];
    if (entry.hash_key != key || depth >= entry.depth) {
      entry.hash_key = key;
      entry.score = score;
      entry.best_move = move;
      entry.depth = depth;
      entry.flag = flag;
    }
  }

  TTEntry* probe(uint64_t key) {
    size_t index = key & mask;
    TTEntry& entry = table[index];
    if (entry.hash_key == key) {
#ifdef DEBUG
      hits++;
#endif
      return &entry;
    }
    return nullptr;
  }

  void clear() {
#ifdef DEBUG
    hits = 0;
#endif
    std::fill(table.begin(), table.end(), TTEntry());
  }

 private:
  std::vector<TTEntry> table;
  size_t mask;
};
