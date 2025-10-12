// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#include "uci.h"
#include "zobrist.h"

int main (/*int argc, char *argv[]*/) {
  Zobrist::init_zobrist_keys();
  UCIengine engine; engine.loop();
  return 0;
}
