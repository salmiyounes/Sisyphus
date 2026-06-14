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

void gen_curr_state_zobrist(ChessBoard *board) {
  for (int pc = WHITE_PAWN; pc <= BLACK_KING; pc++) {
    bb bbit = board->bb_squares[pc];
    int sq;

    while (bbit) {
      POP_LSB(sq, bbit);
      board->hash ^= HASH_PIECES[pc][sq];
    }
  }

  if (board->color) {
    board->hash ^= HASH_COLOR_SIDE;
  }

  board->hash ^= HASH_CASTLE[board->castle];

  if (board->ep) {
    board->hash ^= HASH_EP[get_lsb(board->ep) % 8];
  }
}

void gen_pawn_zobrist(ChessBoard *board) {

  for (int pc = WHITE_PAWN; pc <= BLACK_PAWN; pc++) {
    bb bbit = board->bb_squares[pc];
    int sq;
    while (bbit) {
      POP_LSB(sq, bbit);
      board->pawn_hash ^= HASH_PIECES[pc][sq];
    }
  }

  if (board->ep) {
    board->pawn_hash ^= HASH_EP[get_lsb(board->ep) % 8];
  }

  if (board->color) {
    board->pawn_hash ^= HASH_COLOR_SIDE;
  }
}
