#ifndef BB_H
#define BB_H
#include "types.h"
#include <assert.h>
#include <stdio.h>

/* Macros */
#define BIT(sq) (U64(1) << (sq))

#define POP_LSB(b, x)                                                          \
  b = get_lsb(x);                                                              \
  x &= ~BIT(b);
#define POP_MSB(b, x)                                                          \
  b = get_msb(x);                                                              \
  x &= ~BIT(b);

#define SET_BIT(bbit, sq) ((bbit) |= BIT(sq))
#define CLEAR_BIT(bbit, sq) ((bbit) &= ~BIT(sq))

// Pre-calculated move bitboards for each piece type and square
extern bb BB_PAWNS[2][64];
extern bb BB_KNIGHT[64];
extern bb BB_BISHOP[64];
extern bb BB_ROOK[64];
extern bb BB_KING[64];

// Initialize bitboard lookup tables
void bb_init();

// Returns index of least significant bit
int get_lsb(bb bbit);

// Returns index of most significant bit
int get_msb(bb bbit);

// Counts number of set bits in bitboard
int popcount(bb bbit);

// Checks if bitboard has more than one bit set
int several(bb bbit);

// Tests if specific square is set in bitboard
bool test_bit(bb bbit, const int sq);

// Gets file number (0-7) of a square
int file_of(int sq);

// Gets rank number (0-7) of a square
int rank_of(int sq);

// Converts rank and file to square index
int square(int rank, int file);

bb bb_get_bishop_attacks(int sq, bb obs);

bb bb_get_rook_attacks(int sq, bb obs);

int bb_attacks_to_king_square(ChessBoard *board, bb b_king);

bb bb_attacks_to_square(ChessBoard *board, int sq, bb occ);

// Creates piece type combining piece and color
int make_piece_type(int pc, int color);

// Gets bishop attacks from square considering obstacles
bb bb_bishop(int sq, bb obs);

// Gets rook attacks from square considering obstacles
bb bb_rook(int sq, bb obs);

// Gets queen attacks from square considering obstacles
bb bb_queen(int sq, bb obs);

// Prints visual representation of bitboard
void bb_print(bb bbit);

#endif // BB_H
