// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#include <cassert>
#include <iostream>

#include "board.h"
#include "helpers.h"
#include "fen.h"

int main() {
  // Standard starting position FEN
  std::string startFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -";
  Board board = FEN::parse(startFEN);
  assert(board.sideToMove == WHITE);

  std::cout << "Printing parsed board:\n";
  PrintingHelpers::printBoard(board);

  // Convert back to FEN
  std::string fen2 = FEN::to_fen(board);
  std::cout << "FEN after parsing: " << fen2 << "\n";

  // Assert round-trip equivalence (ignoring trailing spaces)
  assert(fen2.substr(0, startFEN.size()) == startFEN);

  // Custom FEN
  std::string customFEN = "r1bqkb1r/ppp2ppp/2n1pn2/3p4/3P4/2N1PN2/PPP2PPP/8 b KQkq -";
  Board customBoard = FEN::parse(customFEN);
  std::cout << "\nCustom board:\n";
  PrintingHelpers::printBoard(customBoard);
  std::string fen3 = FEN::to_fen(customBoard);
  assert(fen3.substr(0, customFEN.size()) == customFEN);

  // Test en passant
  std::string epFEN = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3";
  Board epBoard = FEN::parse(epFEN);
  assert(epBoard.enPassant != NO_SQUARE);
  std::string epFEN2 = FEN::to_fen(epBoard);
  assert(epFEN2.substr(0, epFEN.size()) == epFEN);

  // Test no castling rights
  std::string noCastle = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - -";
  Board noCastleBoard = FEN::parse(noCastle);
  assert(!noCastleBoard.castling.whiteKingside);
  assert(!noCastleBoard.castling.whiteQueenside);

  std::cout << "\nAll tests passed!\n";
  return 0;
}
