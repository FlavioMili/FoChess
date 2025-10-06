// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#include "fen.h"

#include <cstddef>
#include <sstream>

#include "board.h"
#include "types.h"

namespace {

inline Piece charToPiece(char c) {
  switch (std::toupper(c)) {
    case 'P':
      return PAWN;
    case 'N':
      return KNIGHT;
    case 'B':
      return BISHOP;
    case 'R':
      return ROOK;
    case 'Q':
      return QUEEN;
    case 'K':
      return KING;
    default:
      throw std::runtime_error("Invalid FEN piece char");
  }
}

inline Color charToColor(char c) {
  return std::isupper(c) ? WHITE : BLACK;
}

inline char pieceToChar(Piece pt, Color c) {
  char ch = '?';
  switch (pt) {
    case PAWN:
      ch = 'P';
      break;
    case KNIGHT:
      ch = 'N';
      break;
    case BISHOP:
      ch = 'B';
      break;
    case ROOK:
      ch = 'R';
      break;
    case QUEEN:
      ch = 'Q';
      break;
    case KING:
      ch = 'K';
      break;
    default:
      ch = '?';
      break;
  }
  if (c == BLACK) ch = static_cast<char>(std::tolower(ch));
  return ch;
}

}  // anonymous namespace

namespace FEN {

Board parse(const std::string& fen) {
  Board board;

  std::istringstream ss(fen);
  std::string piecePart, sidePart, castlePart, epPart;
  ss >> piecePart >> sidePart >> castlePart >> epPart;

  int sq = 0;
  for (char c : piecePart) {
    if (c == '/') continue;
    if (std::isdigit(c)) {
      sq += c - '0';
    } else {
      Piece pt = charToPiece(c);
      Color col = charToColor(c);
      board.pieces[col][pt] |= (1ULL << sq);
      ++sq;
    }
  }

  board.updateOccupancy();

  board.sideToMove = (sidePart == "w") ? WHITE : BLACK;

  CastlingRights cr;
  for (char c : castlePart) {
    switch (c) {
      case 'K':
        cr.whiteKingside = true;
        break;
      case 'Q':
        cr.whiteQueenside = true;
        break;
      case 'k':
        cr.blackKingside = true;
        break;
      case 'q':
        cr.blackQueenside = true;
        break;
      case '-':
        break;
    }
  }
  board.castling = cr;

  // En Passant processing
  if (epPart == "-") {
    board.enPassant = NO_SQUARE;
  } else {
    char file = epPart[0];
    char rank = epPart[1];
    int f = file - 'a';
    int r = '8' - rank;
    board.enPassant = static_cast<Square>(r * 8 + f);
  }

  return board;
}

std::string to_fen(const Board& board) {
  std::string fen;

  for (int rank = 7; rank >= 0; --rank) {
    int empty = 0;
    for (int file = 0; file < 8; ++file) {
      int sq = (7 - rank) * 8 + file; 
      Bitboard mask = 1ULL << sq;
      bool found = false;
      for (size_t c = 0; c < 2; ++c) {
        for (size_t p = 0; p < 6; ++p) {
          if (board.pieces[c][p] & mask) {
            if (empty > 0) {
              fen += std::to_string(empty);
              empty = 0;
            }
            fen += ::pieceToChar(static_cast<Piece>(p), static_cast<Color>(c));
            found = true;
            break;
          }
        }
        if (found) break;
      }
      if (!found) ++empty;
    }
    if (empty > 0) fen += std::to_string(empty);
    if (rank > 0) fen += '/';
  }

  fen += ' ';
  fen += (board.sideToMove == WHITE) ? 'w' : 'b';

  fen += ' ';
  bool any = false;
  if (board.castling.whiteKingside) {
    fen += 'K';
    any = true;
  }
  if (board.castling.whiteQueenside) {
    fen += 'Q';
    any = true;
  }
  if (board.castling.blackKingside) {
    fen += 'k';
    any = true;
  }
  if (board.castling.blackQueenside) {
    fen += 'q';
    any = true;
  }
  if (!any) fen += '-';

  // En Croissant processing
  fen += ' ';
  if (board.enPassant == NO_SQUARE)
    fen += '-';
  else {
    int r = board.enPassant / 8;
    int f = board.enPassant % 8;
    fen += static_cast<char>('a' + f);
    fen += static_cast<char>('8' - r);
  }

  return fen;
}

}  // namespace FEN
