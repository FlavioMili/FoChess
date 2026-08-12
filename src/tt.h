// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#pragma once

#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
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

  TTEntry()
      : hash_key(0),
        score(0),
        best_move(Move::none()),
        depth(0),
        flag(TT_NONE) {}
};

struct alignas(64) TTCluster {
  std::array<TTEntry, 4> entries;
};

static_assert(sizeof(TTEntry) == 16);
static_assert(sizeof(TTCluster) == 64);

class TranspositionTable {
 public:
#ifdef DEBUG
  int hits = 0;
#endif
  explicit TranspositionTable(size_t mb_size = 64) {
    const size_t requested_clusters = std::max<size_t>(
        1, (mb_size * 1024 * 1024) / sizeof(TTCluster));
    const size_t cluster_count = std::bit_floor(requested_clusters);
    table.resize(cluster_count);
    mask = cluster_count - 1;
  }

  void store(uint64_t key, int score, Move move, uint8_t depth, TTFlag flag) {
    TTCluster& cluster = table[key & mask];
    TTEntry* replacement = &cluster.entries[0];

    for (TTEntry& entry : cluster.entries) {
      if (entry.hash_key == key && entry.flag != TT_NONE) {
        if (depth >= entry.depth) write(entry, key, score, move, depth, flag);
        return;
      }
      if (entry.flag == TT_NONE) {
        replacement = &entry;
        break;
      }
      if (entry.depth < replacement->depth) replacement = &entry;
    }

    write(*replacement, key, score, move, depth, flag);
  }

  // Quiescence entries may use an empty slot or refresh another quiescence
  // entry, but never evict a deeper principal-search result.
  void store_quiescence(uint64_t key, int score, Move move, TTFlag flag) {
    TTCluster& cluster = table[key & mask];
    TTEntry* empty = nullptr;

    for (TTEntry& entry : cluster.entries) {
      if (entry.hash_key == key && entry.flag != TT_NONE) {
        if (entry.depth == 0) write(entry, key, score, move, 0, flag);
        return;
      }
      if (entry.flag == TT_NONE && empty == nullptr) empty = &entry;
    }

    if (empty != nullptr) write(*empty, key, score, move, 0, flag);
  }

  TTEntry* probe(uint64_t key) {
    TTCluster& cluster = table[key & mask];
    for (TTEntry& entry : cluster.entries) {
      if (entry.hash_key != key || entry.flag == TT_NONE) continue;
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
    std::fill(table.begin(), table.end(), TTCluster());
  }

 private:
  static void write(TTEntry& entry, uint64_t key, int score, Move move,
                    uint8_t depth, TTFlag flag) {
    entry.hash_key = key;
    entry.score = score;
    entry.best_move = move;
    entry.depth = depth;
    entry.flag = flag;
  }

  std::vector<TTCluster> table;
  size_t mask;
};
