#ifndef BB_H
#define BB_H
#include "types.h"
#include <assert.h>

/* Macros */
#define EMPTY_BB U64(0)
#define BIT(sq) (U64(1) << (sq))

#define POP_LSB(b, x)                                                          \
  b = get_lsb(x);                                                              \
  x &= ~BIT(b);
#define POP_MSB(b, x)                                                          \
  b = get_msb(x);                                                              \
  x &= ~BIT(b);

#define SET_BIT(bbit, sq) ((bbit) |= BIT(sq))
#define CLEAR_BIT(bbit, sq) ((bbit) &= ~BIT(sq))

#define lsb(x) get_lsb(x)
#define msb(x) get_msb(x)

// Pre-calculated move bitboards for each piece type and square
extern bb BB_PAWNS[2][64];
extern bb BB_KNIGHT[64];
extern bb BB_BISHOP[64];
extern bb BB_ROOK[64];
extern bb BB_KING[64];

// Magic bitboard tables (defined in bb.c, filled by bb_init)
extern const bb MAGIC_BISHOP[64];
extern const bb MAGIC_ROOK[64];
extern const int SHIFT_BISHOP[64];
extern const int SHIFT_ROOK[64];
extern int OFFSET_BISHOP[64];
extern int OFFSET_ROOK[64];
extern bb ATTACK_BISHOP[5248];
extern bb ATTACK_ROOK[102400];

// Initialize bitboard lookup tables
void bb_init();

// Returns index of least significant bit (bbit must be non-zero)
INLINE int get_lsb(bb bbit) {
  ASSERT(bbit);
  return __builtin_ctzll(bbit);
}

// Returns index of most significant bit (bbit must be non-zero)
INLINE int get_msb(bb bbit) {
  ASSERT(bbit);
  return __builtin_clzll(bbit) ^ 63;
}

// Counts number of set bits in bitboard
INLINE int popcount(bb bbit) { return __builtin_popcountll(bbit); }

// Checks if bitboard has more than one bit set
INLINE bool several(bb bbit) { return (bbit & (bbit - 1)) != 0; }

// Tests if specific square is set in bitboard
INLINE bool test_bit(bb bbit, const enum Square sq) {
  ASSERT(sq >= 0 && sq < SQUARE_NB);
  return (bool)(bbit & BIT(sq));
}

INLINE enum Color piece_color(enum ColoredPiece piece) {
  return (enum Color)(piece & 1);
}

INLINE enum PieceType piece_type(enum ColoredPiece piece) {
  return (enum PieceType)(piece >> 1);
}

INLINE enum Square flip(enum Square square) {
  return (enum Square)(square ^ 56);
}

INLINE enum Square flip_63(enum Square square) {
  return (enum Square)(square ^ 63);
}

// Gets file number (0-7) of a square
INLINE int file_of(enum Square sq) {
  ASSERT(0 <= sq && sq < SQUARE_NB);
  return sq & 7;
}

// Gets rank number (0-7) of a square
INLINE int rank_of(enum Square sq) {
  ASSERT(0 <= sq && sq < SQUARE_NB);
  return sq >> 3;
}

// Converts rank and file to square index
INLINE int square(int rank, int file) {
  ASSERT(0 <= rank && rank < RANK_NB);
  ASSERT(0 <= file && file < FILE_NB);
  return rank * FILE_NB + file;
}

// Creates piece type combining piece and color
INLINE int make_piece_type(enum PieceType pc, enum Color color) {
  ASSERT(color == WHITE || color == BLACK);
  ASSERT(pc >= PAWN && pc <= KING);
  return (pc << 1) + color;
}

// Gets bishop attacks from square considering obstacles
INLINE bb bb_bishop(enum Square sq, bb obs) {
  bb value = obs & BB_BISHOP[sq];
  int index = (int)((value * MAGIC_BISHOP[sq]) >> SHIFT_BISHOP[sq]);
  return ATTACK_BISHOP[index + OFFSET_BISHOP[sq]];
}

// Gets rook attacks from square considering obstacles
INLINE bb bb_rook(enum Square sq, bb obs) {
  bb value = obs & BB_ROOK[sq];
  int index = (int)((value * MAGIC_ROOK[sq]) >> SHIFT_ROOK[sq]);
  return ATTACK_ROOK[index + OFFSET_ROOK[sq]];
}

// Gets queen attacks from square considering obstacles
INLINE bb bb_queen(enum Square sq, bb obs) {
  return bb_bishop(sq, obs) | bb_rook(sq, obs);
}

bb bb_get_bishop_attacks(enum Square sq, bb obs);

bb bb_get_rook_attacks(enum Square sq, bb obs);

int bb_attacks_to_king_square(ChessBoard *board, bb b_king);

bool bb_is_check(ChessBoard *board);

bool is_square_attacked_by(ChessBoard *board, enum Square square,
                           enum Color color);

bb bb_attacks_to_square(ChessBoard *board, enum Square sq, bb occ);

bb bb_pawns_attacks(enum Square sq, enum Color color);

bb bb_get_knight_attacks(enum Square sq);

// Prints visual representation of bitboard
void bb_print(bb bbit);

#endif // BB_H
