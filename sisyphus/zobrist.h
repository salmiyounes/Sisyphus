#ifndef ZOBRIST_H
#define ZOBRIST_H

#include "bb.h"
#include "types.h"
#include <stdio.h>

// Random hash values for each piece on each square
extern bb HASH_PIECES[12][64];

// Hash values for en passant squares
extern bb HASH_EP[8];

// Hash values for castling rights
extern bb HASH_CASTLE[16];

// Hash value for side to move
extern bb HASH_COLOR_SIDE;

// Initialize Zobrist hash tables with random values
void init_zobrist();

// Generate Zobrist hash for current board state
bb gen_curr_state_zobrist(ChessBoard *board);

// Generate Zobrist hash for pawn structure
bb gen_pawn_zobrist(ChessBoard *board);

#endif