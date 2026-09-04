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
#define CASTLE_FLAG 1        // Castling move
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
#define IS_CAS(flag) ((flag) == CASTLE_FLAG)   // Check if castling

// Board update macros
#define TOGGLE_HASH(board)                                                     \
  if (board->castle)                                                           \
    board->hash ^= HASH_CASTLE[board->castle];                                 \
  if (board->ep)                                                               \
    board->hash ^= HASH_EP[file_of(get_lsb(board->ep))];

#define place_promo_piece(board, dst, flag, color)                             \
  board_update((board), (dst), make_piece_type(PROMO_PT(flag), (color)))

// Move encoding format (32 bits):
// from (6 bits) | to (6 bits) | piece (4 bits) | flags (4 bits)
static INLINE Move encode_move(enum Square from, enum Square to,
                               enum ColoredPiece piece, int flag) {
  return (Move)(((from) | ((to) << 6) | ((piece) << 12) | ((flag) << 16)));
}

static INLINE enum Square extract_from(const Move move) {
  return (enum Square)((move >> 0) & 0x3f);
}

static INLINE enum Square extract_to(const Move move) {
  return (enum Square)((move >> 6) & 0x3f);
}

static INLINE enum ColoredPiece extract_piece(const Move move) {
  return (enum ColoredPiece)((move >> 12) & 0xf);
}

static INLINE int extract_flags(const Move move) {
  return (int)((move >> 16) & 0xf);
}

static INLINE void emit_move(Move **moves, enum Square from, enum Square to,
                             enum ColoredPiece piece, int flag) {
  *(*moves)++ = encode_move(from, to, piece, flag);
}

// Helper functions for special moves

static INLINE void handle_white_pawn(ChessBoard *board, enum Square src,
                                     enum Square dst, const int flag,
                                     enum Color color) {
  bb bsrc = BIT(src);
  bb bdst = BIT(dst);
  if ((bsrc & RANK_2) && (bdst & RANK_4))
    board->ep = BIT(src + 8);

  if (IS_ENP(flag))
    board_update(board, dst - 8, NONE);

  if (IS_PROMO(flag))
    place_promo_piece(board, dst, flag, color);
}

static INLINE void unmake_white_pawn(ChessBoard *board, enum Square dst,
                                     const int flag) {
  if (!IS_ENP(flag))
    return;
  board_update(board, dst - 8, BLACK_PAWN);
}

static INLINE void handle_black_pawn(ChessBoard *board, enum Square src,
                                     enum Square dst, const int flag,
                                     enum Color color) {
  bb bsrc = BIT(src);
  bb bdst = BIT(dst);
  if ((bsrc & RANK_7) && (bdst & RANK_5))
    board->ep = BIT(src - 8);

  if (IS_ENP(flag))
    board_update(board, dst + 8, NONE);

  if (IS_PROMO(flag))
    place_promo_piece(board, dst, flag, color);
}

static INLINE void unmake_black_pawn(ChessBoard *board, enum Square dst,
                                     const int flag) {
  if (!IS_ENP(flag))
    return;
  board_update(board, dst + 8, WHITE_PAWN);
}

static INLINE void handle_white_king(ChessBoard *board, enum Square src,
                                     enum Square dst, const int flag) {
  board->castle &= ~CASTLE_WHITE;

  if (!IS_CAS(flag))
    return;

  if (src == E1 && dst == G1) {
    board_update(board, H1, NONE);
    board_update(board, F1, WHITE_ROOK);
  } else if (src == E1 && dst == C1) {
    board_update(board, A1, NONE);
    board_update(board, D1, WHITE_ROOK);
  }
}

static INLINE void unmake_white_king(ChessBoard *board, enum Square src,
                                     enum Square dst, const int flag) {
  if (!IS_CAS(flag))
    return;

  if (src == E1 && dst == G1) {
    board_update(board, H1, WHITE_ROOK);
    board_update(board, F1, NONE);
  } else if (src == E1 && dst == C1) {
    board_update(board, A1, WHITE_ROOK);
    board_update(board, D1, NONE);
  }
}

static INLINE void handle_black_king(ChessBoard *board, enum Square src,
                                     enum Square dst, const int flag) {
  board->castle &= ~CASTLE_BLACK;

  if (!IS_CAS(flag))
    return;

  if (src == E8 && dst == G8) {
    board_update(board, H8, NONE);
    board_update(board, F8, BLACK_ROOK);
  } else if (src == E8 && dst == C8) {
    board_update(board, A8, NONE);
    board_update(board, D8, BLACK_ROOK);
  }
}

static INLINE void unmake_black_king(ChessBoard *board, enum Square src,
                                     enum Square dst, const int flag) {
  if (!IS_CAS(flag))
    return;

  if (src == E8 && dst == G8) {
    board_update(board, H8, BLACK_ROOK);
    board_update(board, F8, NONE);
  } else if (src == E8 && dst == C8) {
    board_update(board, A8, BLACK_ROOK);
    board_update(board, D8, NONE);
  }
}

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