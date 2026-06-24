#ifndef BOARD_H
#define BOARD_H

#include "bb.h"
#include "search.h"
#include "thpool.h"
#include "types.h"
#include "zobrist.h"
#include <stdbool.h>
#include <stdio.h>

// Piece square tables for positional evaluation
extern const int pawn_square_values[64];
extern const int knight_square_values[64];
extern const int bishop_square_values[64];
extern const int rook_square_values[64];
extern const int king_square_values[64];
extern const int queen_square_values[64];

// Material values for middle and endgame phases
extern int mg_value[6];
extern int eg_value[6];

// Piece square tables for middle and endgame phases
extern int mg_pawn_table[64];
extern int eg_pawn_table[64];
extern int mg_knight_table[64];
extern int eg_knight_table[64];
extern int mg_bishop_table[64];
extern int eg_bishop_table[64];
extern int mg_rook_table[64];
extern int eg_rook_table[64];
extern int mg_queen_table[64];
extern int eg_queen_table[64];
extern int mg_king_table[64];
extern int eg_king_table[64];

// PESTO evaluation tables
extern int *mg_pesto_table[6];
extern int *eg_pesto_table[6];
extern int mg_table[12][64];
extern int eg_table[12][64];

// Game phase weights for each piece
extern int gamephaseInc[12];

// Lookup tables for piece values and castling
extern const int *square_values[13];
extern int piece_material[13];
extern int castling_rights[64];

/* Macros */
#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define MAX(A, B) ((A) > (B) ? (A) : (B))
#define FLIP(sq) ((sq ^ 56))               // Flip square vertically
#define FLIP_63(sq) ((sq ^ 63))            // Flip square diagonally
#define PIECE(x) (((x) & ~1) >> 1)         // Get piece type from piece code
#define COLOR(x) ((x) & 1)                 // Get piece color
#define SWITCH_SIDE(x) (x->color ^= BLACK) // Switch side to move

// Board manipulation functions
void board_init(ChessBoard *b);  // Initialize chess board
void print_board(ChessBoard *b); // Print board representation
void board_load_fen(ChessBoard *board,
                    const char *fen);            // Load position from FEN
void board_to_fen(ChessBoard *board, char *fen); // Convert position to FEN
void board_update(ChessBoard *board, int sq, int piece); // Update board state

// Game state evaluation
int board_drawn_by_insufficient_material(
    ChessBoard *board);                  // Check material draw
int is_draw(ChessBoard *board, int ply); // Check if position is drawn

// Testing and threading
bb perft_test(ChessBoard *board, int depth); // Performance test
void thread_init(Search *search, ChessBoard *board, Move *result,
                 float duration, bool debug); // Initialize search thread
void thread_stop(Search *search);             // Stop search thread

#endif // BOARD_H