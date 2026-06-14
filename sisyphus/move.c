#include "move.h"

int History_Heuristic[COLOR_NB][SQUARE_NB][SQUARE_NB] = {0};
Move KILLER_MOVES[COLOR_NB][64] = {0};

const char *PROMOTION_TO_CHAR = "-nbrq-";

// from https://rustic-chess.org/search/ordering/mvv_lva.html
const int MVV_LVA[6][6] = {
    [KING] = {0, 0, 0, 0, 0, 0},         [QUEEN] = {55, 54, 53, 52, 51, 50},
    [ROOK] = {45, 44, 43, 42, 41, 40},   [BISHOP] = {35, 34, 33, 32, 31, 30},
    [KNIGHT] = {25, 24, 23, 22, 21, 20}, [PAWN] = {15, 14, 13, 12, 11, 10},
};

const char *SQ_TO_COORD[64] = {
    "h8", "g8", "f8", "e8", "d8", "c8", "b8", "a8", "h7", "g7", "f7",
    "e7", "d7", "c7", "b7", "a7", "h6", "g6", "f6", "e6", "d6", "c6",
    "b6", "a6", "h5", "g5", "f5", "e5", "d5", "c5", "b5", "a5", "h4",
    "g4", "f4", "e4", "d4", "c4", "b4", "a4", "h3", "g3", "f3", "e3",
    "d3", "c3", "b3", "a3", "h2", "g2", "f2", "e2", "d2", "c2", "b2",
    "a2", "h1", "g1", "f1", "e1", "d1", "c1", "b1", "a1",
};

void make_move(ChessBoard *board, Move move) {
  Undo undo;
  do_move(board, move, &undo);
}

bool is_capture(ChessBoard *board, const Move move) {
  return (bool)(board->squares[EXTRACT_TO(move)] != NONE);
}

bool is_tactical_move(ChessBoard *board, const Move move) {
  int flag = EXTRACT_FLAGS(move);
  return is_capture(board, move) || IS_ENP(flag) || IS_PROMO(flag);
}

void do_move(ChessBoard *board, Move move, Undo *undo) {
  int src = EXTRACT_FROM(move);
  int dst = EXTRACT_TO(move);
  int piece = EXTRACT_PIECE(move);
  int color = COLOR(piece);
  int flag = EXTRACT_FLAGS(move);

  ASSERT((src >= 0 && src < SQUARE_NB) && (dst >= 0 && dst < SQUARE_NB));
  ASSERT(piece >= WHITE_PAWN && piece <= NONE);
  ASSERT(color == WHITE || color == BLACK);
  ASSERT(flag >= EMPTY_FLAG && flag <= QUEEN_PROMO_FLAG);

  board->m_history[board->numMoves++] = board->hash;

  TOGGLE_HASH(board);

  undo->capture = board->squares[dst];
  undo->ep = board->ep;
  undo->castle = board->castle;

  board_update(board, src, NONE);

  if (!IS_PROMO(flag)) {
    board_update(board, dst, piece);
  }

  board->ep = U64(0);

  if (piece == WHITE_PAWN) {
    bb bsrc = BIT(src);
    bb bdst = BIT(dst);
    if ((bsrc & RANK_2) && (bdst & RANK_4)) {
      board->ep = BIT(src + 8);
    }
    if (IS_ENP(flag)) {
      board_update(board, dst - 8, NONE);
    }
    HANDLE_PROMOTION(board, piece, flag, dst, color);
  } else if (piece == BLACK_PAWN) {
    bb bsrc = BIT(src);
    bb bdst = BIT(dst);
    if ((bsrc & RANK_7) && (bdst & RANK_5)) {
      board->ep = BIT(src - 8);
    }
    if (IS_ENP(flag)) {
      board_update(board, dst + 8, NONE);
    }
    HANDLE_PROMOTION(board, piece, flag, dst, color);
  } else if (piece == WHITE_KING) {
    board->castle &= ~CASTLE_WHITE;
    if (IS_CAS(flag)) {
      if (src == 4 && dst == 6) {
        board_update(board, 7, NONE);
        board_update(board, 5, WHITE_ROOK);
      } else if (src == 4 && dst == 2) {
        board_update(board, 0, NONE);
        board_update(board, 3, WHITE_ROOK);
      }
    }
  } else if (piece == BLACK_KING) {
    board->castle &= ~CASTLE_BLACK;
    if (IS_CAS(flag)) {
      if (src == 60 && dst == 62) {
        board_update(board, 63, NONE);
        board_update(board, 61, BLACK_ROOK);
      } else if (src == 60 && dst == 58) {
        board_update(board, 56, NONE);
        board_update(board, 59, BLACK_ROOK);
      }
    }
  }

  if (board->castle) {
    board->castle &= ~castling_rights[src];
    board->castle &= ~castling_rights[dst];
  }

  SWITCH_SIDE(board);
  board->hash ^= HASH_COLOR_SIDE;
  TOGGLE_HASH(board);
}

void undo_move(ChessBoard *board, Move move, Undo *undo) {
  int piece = EXTRACT_PIECE(move);
  int src = EXTRACT_FROM(move);
  int dst = EXTRACT_TO(move);
  int flag = EXTRACT_FLAGS(move);

  ASSERT((src >= 0 && src < SQUARE_NB) && (dst >= 0 && dst < SQUARE_NB));
  ASSERT(piece >= WHITE_PAWN && piece <= NONE);
  ASSERT(flag >= EMPTY_FLAG && flag <= QUEEN_PROMO_FLAG);

  TOGGLE_HASH(board);

  int capture = undo->capture;

  board->ep = undo->ep;
  board->castle = undo->castle;

  board_update(board, src, piece);

  board_update(board, dst, capture);

  if (piece == WHITE_PAWN) {
    if (IS_ENP(flag)) {
      board_update(board, dst - 8, BLACK_PAWN);
    }
  } else if (piece == BLACK_PAWN) {
    if (IS_ENP(flag)) {
      board_update(board, dst + 8, WHITE_PAWN);
    }
  }

  else if (piece == WHITE_KING) {
    if (IS_CAS(flag)) {
      if (src == 4 && dst == 6) {
        board_update(board, 7, WHITE_ROOK);
        board_update(board, 5, NONE);
      } else if (src == 4 && dst == 2) {
        board_update(board, 0, WHITE_ROOK);
        board_update(board, 3, NONE);
      }
    }
  } else if (piece == BLACK_KING) {
    if (IS_CAS(flag)) {
      if (src == 60 && dst == 62) {
        board_update(board, 63, BLACK_ROOK);
        board_update(board, 61, NONE);
      } else if (src == 60 && dst == 58) {
        board_update(board, 56, BLACK_ROOK);
        board_update(board, 59, NONE);
      }
    }
  }
  SWITCH_SIDE(board);
  board->hash ^= HASH_COLOR_SIDE;
  TOGGLE_HASH(board);
  board->numMoves--;
}

void do_null_move_pruning(ChessBoard *board, Undo *undo) {
  board->m_history[board->numMoves++] = board->hash;
  TOGGLE_HASH(board);
  undo->ep = board->ep;
  board->ep = U64(0);
  SWITCH_SIDE(board);
  board->hash ^= HASH_COLOR_SIDE;
  TOGGLE_HASH(board);
}

void undo_null_move_pruning(ChessBoard *board, Undo *undo) {
  TOGGLE_HASH(board);
  board->ep = undo->ep;
  SWITCH_SIDE(board);
  board->hash ^= HASH_COLOR_SIDE;
  TOGGLE_HASH(board);
  board->numMoves--;
}

void score_moves(ChessBoard *board, Move move, int *score) {
  int piece = EXTRACT_PIECE(move);
  int src = EXTRACT_FROM(move), dst = EXTRACT_TO(move),
      flag = EXTRACT_FLAGS(move);
  int color = COLOR(piece), result = 0;
  int attacker = PIECE(EXTRACT_PIECE(move)),
      victim = PIECE(board->squares[dst]);

  result =
      (color == WHITE)
          ? square_values[piece][(dst)] - square_values[piece][(src)]
          : square_values[piece][FLIP(dst)] - square_values[piece][FLIP(src)];

  if (is_capture(board, move))
    result += MVV_LVA[victim][attacker] + 10000;

  if (IS_PROMO(flag))
    result += (color == WHITE) ? square_values[PROMO_PT(flag)][dst] -
                                     square_values[WHITE_PAWN][dst]
                               : square_values[PROMO_PT(flag)][FLIP(dst)] -
                                     square_values[BLACK_PAWN][FLIP(dst)];
  else if (IS_ENP(flag))
    result += MVV_LVA[PAWN][PAWN] + 10000;

  *score = result;
}

INLINE int move_estimated_value(ChessBoard *board, Move move) {
  int flag = EXTRACT_FLAGS(move);
  int value = SEEPieceValues[PIECE(board->squares[EXTRACT_TO(move)])];

  if (IS_PROMO(flag))
    value += SEEPieceValues[PROMO_PT(flag)] - SEEPieceValues[PAWN];

  else if (IS_ENP(flag))
    value = SEEPieceValues[PAWN];

  else if (IS_CAS(flag))
    value = 0;

  return value;
}

char *move_to_str(Move move) {
  static char buffer[6];
  int src = EXTRACT_FROM(move), dst = EXTRACT_TO(move);
  int flag = EXTRACT_FLAGS(move);
  if (IS_PROMO(flag)) {
    sprintf(buffer, "%s%s%c", SQ_TO_COORD[FLIP_63(src)],
            SQ_TO_COORD[FLIP_63(dst)], PROMOTION_TO_CHAR[PROMO_PT(flag)]);
  } else {
    sprintf(buffer, "%s%s", SQ_TO_COORD[FLIP_63(src)],
            SQ_TO_COORD[FLIP_63(dst)]);
  }
  return buffer;
}