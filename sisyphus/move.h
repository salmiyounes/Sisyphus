#ifndef MOVE_H
#define MOVE_H

#include "bb.h"
#include "board.h"
#include "gen.h"
#include "zobrist.h"
#include <assert.h>
#include <stdbool.h>

// Constants
#define MAX_MOVES 218 // Maximum possible moves in any position

// Move flags for different types of moves
#define EMPTY_FLAG 0         // Normal move
#define ENP_FLAG 6           // En passant capture
#define CAPTURE_FLAG 4       // Regular capture
#define CATLE_FLAG 1         // Castling move
#define PROMO_FLAG 8         // Base promotion flag
#define KNIGHT_PROMO_FLAG 8  // Promote to knight
#define ROOK_PROMO_FLAG 9    // Promote to rook
#define BISHOP_PROMO_FLAG 10 // Promote to bishop
#define QUEEN_PROMO_FLAG 11  // Promote to queen

// Move flag checking macros
#define NULL_MOVE U32(0) // Represents no move
#define IS_PROMO(flag)                                                         \
  ((bool)((flag) & PROMO_FLAG))                // Check if move is promotion
#define PROMO_PT(flag) ((flag & 0x3) + KNIGHT) // Get promotion piece type
#define IS_ENP(flag) ((flag) == ENP_FLAG)      // Check if en passant
#define IS_CAS(flag) ((flag) == CATLE_FLAG)    // Check if castling

// Board update macros
#define HANDLE_PROMOTION(board, piece, flag, dst, color)                       \
  if (IS_PROMO(flag)) {                                                        \
    board_update(board, dst, make_piece_type(PROMO_PT(flag), color));          \
  }

#define TOGGLE_HASH(board)                                                     \
  if (board->castle)                                                           \
    board->hash ^= HASH_CASTLE[board->castle];                                 \
  if (board->ep)                                                               \
    board->hash ^= HASH_EP[get_lsb(board->ep) % 8];

// Move encoding format (32 bits):
// from (6 bits) | to (6 bits) | piece (4 bits) | flags (4 bits)
#define ENCODE_MOVE(from, to, piece, flag)                                     \
  (((from) | ((to) << 6) | ((piece) << 12) | ((flag) << 16)))

// Move decoding macros
#define EXTRACT_FROM(move) ((int)(((move) >> 0) & 0x3f))  // Get source square
#define EXTRACT_TO(move) ((int)(((move) >> 6) & 0x3f))    // Get target square
#define EXTRACT_FLAGS(move) ((int)(((move) >> 16) & 0xf)) // Get move flags
#define EXTRACT_PIECE(move) ((int)(((move) >> 12) & 0xf)) // Get piece type

// Move scoring tables
extern int History_Heuristic[COLOR_NB][SQUARE_NB][SQUARE_NB];
extern Move KILLER_MOVES[COLOR_NB][SQUARE_NB];
extern const int MVV_LVA[6][6];
static const int SEEPieceValues[] = {
    103, 422, 437, 694, 1313, 0, 0, 0,
};

// Move manipulation functions
void do_move(ChessBoard *board, Move move, Undo *undo); // Make a move
void make_move(ChessBoard *board, Move move); // Make move without undo
void undo_move(ChessBoard *board, Move move, Undo *undo); // Take back a move

// Move scoring and evaluation
void score_moves(ChessBoard *board, Move move,
                 int *score); // Score moves for ordering
int move_estimated_value(ChessBoard *board, Move move); // Estimate move value

// Null move pruning
void undo_null_move_pruning(ChessBoard *board, Undo *undo); // Undo null move
void do_null_move_pruning(ChessBoard *board, Undo *undo);   // Make null move

// Utility functions
char *move_to_str(Move move);                        // Convert move to string
bool is_capture(ChessBoard *board, const Move move); // Check if move is capture
bool is_tactical_move(ChessBoard *board,
                      const Move move); // Check if tactical move

#endif // MOVE_H