// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#include "magic.h"
#include "uci.h"
#include "zobrist.h"

int main (/*int argc, char *argv[]*/) {
  Zobrist::init_zobrist_keys();
  Bitboards::init_magic_tables();
  UCIengine engine; engine.loop();
  return 0;
}
