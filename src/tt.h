#pragma once

#include <sys/types.h>

#include <cstdint>
#include <vector>

#include "bitboard.h"
#include "move.h"

constexpr size_t TTSize = 1 << 2;

struct TTFlags {
  uint8_t exact : 1;
  uint8_t lowerbound : 1;
  uint8_t upperbound : 1;
  uint8_t unused : 5;

  constexpr TTFlags() : exact(0), lowerbound(0), upperbound(0), unused(0) {}
  constexpr TTFlags(bool e, bool l, bool u) : exact(e), lowerbound(l), upperbound(u), unused(0) {}
};

struct TTEntry {
  Bitboard hash_key;
  int score;
  Move move;
  uint8_t depth;
  TTFlags flags;
};

class TranspositionTable {
 public:
  explicit TranspositionTable(size_t size = 1 << 20) : TT(size) {}

  void store(Bitboard key, int score, Move bestMove, uint8_t depth, TTFlags flag) {
    size_t idx = key & (TT.size() - 1);
    TT[idx] = {key, score, bestMove, depth, flag};
  }

  TTEntry* probe(Bitboard key) {
    TTEntry& entry = TT[key & (TT.size() - 1)];
    return (entry.hash_key == key) ? &entry : nullptr;
  }

  void clear() {
    std::fill(TT.begin(), TT.end(), TTEntry{});
  }

 private:
  std::vector<TTEntry> TT;
};
