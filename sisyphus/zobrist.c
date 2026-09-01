#include "zobrist.h"
#include "utils.h"

bb HASH_PIECES[12][64];
bb HASH_EP[8];
bb HASH_CASTLE[16];
bb HASH_COLOR_SIDE;

void init_zobrist() {
  for (int i = 0; i < 12; i++) {
    for (int j = 0; j < 64; j++) {
      HASH_PIECES[i][j] = xorshift64();
    }
  }

  for (int i = 0; i < 8; i++) {
    HASH_EP[i] = xorshift64();
  }

  for (int i = 0; i < 16; i++) {
    HASH_CASTLE[i] = xorshift64();
  }
  HASH_COLOR_SIDE = xorshift64();
}

bb gen_curr_state_zobrist(ChessBoard *board) {
  bb hash = EMPTY_BB;
  for (int pc = WHITE_PAWN; pc <= BLACK_KING; pc++) {
    bb bbit = board->bb_squares[pc];
    int sq;

    while (bbit) {
      POP_LSB(sq, bbit);
      hash ^= HASH_PIECES[pc][sq];
    }
  }

  if (board->color) {
    hash ^= HASH_COLOR_SIDE;
  }

  hash ^= HASH_CASTLE[board->castle];

  if (board->ep) {
    hash ^= HASH_EP[get_lsb(board->ep) % 8];
  }

  return hash;
}

bb gen_pawn_zobrist(ChessBoard *board) {
  bb hash = EMPTY_BB;
  for (int pc = WHITE_PAWN; pc <= BLACK_PAWN; pc++) {
    bb bbit = board->bb_squares[pc];
    int sq;
    while (bbit) {
      POP_LSB(sq, bbit);
      hash ^= HASH_PIECES[pc][sq];
    }
  }

  if (board->ep) {
    hash ^= HASH_EP[get_lsb(board->ep) % 8];
  }

  if (board->color) {
    hash ^= HASH_COLOR_SIDE;
  }

  return hash;
}
