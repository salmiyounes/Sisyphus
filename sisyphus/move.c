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
  return (bool)(board->squares[extract_to(move)] != NONE);
}

bool is_tactical_move(ChessBoard *board, const Move move) {
  int flag = extract_flags(move);
  return is_capture(board, move) || IS_ENP(flag) || IS_PROMO(flag);
}

INLINE void do_move(ChessBoard *board, Move move, Undo *undo) {
  int src = extract_from(move);
  int dst = extract_to(move);
  int piece = extract_piece(move);
  int color = piece_color(piece);
  int flag = extract_flags(move);

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

  if (!IS_PROMO(flag))
    board_update(board, dst, piece);

  board->ep = EMPTY_BB;

  switch (piece) {
  case WHITE_PAWN: {
    bb bsrc = BIT(src);
    bb bdst = BIT(dst);
    if ((bsrc & RANK_2) && (bdst & RANK_4)) {
      board->ep = BIT(src + 8);
    }
    if (IS_ENP(flag)) {
      board_update(board, dst - 8, NONE);
    }
    HANDLE_PROMOTION(board, piece, flag, dst, color);
    break;
  }
  case BLACK_PAWN: {
    bb bsrc = BIT(src);
    bb bdst = BIT(dst);
    if ((bsrc & RANK_7) && (bdst & RANK_5)) {
      board->ep = BIT(src - 8);
    }
    if (IS_ENP(flag)) {
      board_update(board, dst + 8, NONE);
    }
    HANDLE_PROMOTION(board, piece, flag, dst, color);
    break;
  }
  case WHITE_KING:
    board->castle &= ~CASTLE_WHITE;
    if (IS_CAS(flag)) {
      if (src == E1 && dst == G1) {
        board_update(board, H1, NONE);
        board_update(board, F1, WHITE_ROOK);
      } else if (src == E1 && dst == C1) {
        board_update(board, A1, NONE);
        board_update(board, D1, WHITE_ROOK);
      }
    }
    break;
  case BLACK_KING:
    board->castle &= ~CASTLE_BLACK;
    if (IS_CAS(flag)) {
      if (src == E8 && dst == G8) {
        board_update(board, H8, NONE);
        board_update(board, F8, BLACK_ROOK);
      } else if (src == E8 && dst == C8) {
        board_update(board, A8, NONE);
        board_update(board, D8, BLACK_ROOK);
      }
    }
    break;
  }
  // Update the castling rights
  update_castling_rights(board, src, dst);

  SWITCH_SIDE(board);
  board->hash ^= HASH_COLOR_SIDE;
  TOGGLE_HASH(board);
}

INLINE void undo_move(ChessBoard *board, Move move, Undo *undo) {
  int piece = extract_piece(move);
  int src = extract_from(move);
  int dst = extract_to(move);
  int flag = extract_flags(move);

  ASSERT((src >= 0 && src < SQUARE_NB) && (dst >= 0 && dst < SQUARE_NB));
  ASSERT(piece >= WHITE_PAWN && piece <= NONE);
  ASSERT(flag >= EMPTY_FLAG && flag <= QUEEN_PROMO_FLAG);

  TOGGLE_HASH(board);

  int capture = undo->capture;

  board->ep = undo->ep;
  board->castle = undo->castle;

  board_update(board, src, piece);
  board_update(board, dst, capture);

  switch (piece) {
  case WHITE_PAWN:
    if (IS_ENP(flag)) {
      board_update(board, dst - 8, BLACK_PAWN);
    }
    break;
  case BLACK_PAWN:
    if (IS_ENP(flag)) {
      board_update(board, dst + 8, WHITE_PAWN);
    }
    break;
  case WHITE_KING:
    if (IS_CAS(flag)) {
      if (src == E1 && dst == G1) {
        board_update(board, H1, WHITE_ROOK);
        board_update(board, F1, NONE);
      } else if (src == E1 && dst == C1) {
        board_update(board, A1, WHITE_ROOK);
        board_update(board, D1, NONE);
      }
    }
    break;
  case BLACK_KING:
    if (IS_CAS(flag)) {
      if (src == E8 && dst == G8) {
        board_update(board, H8, BLACK_ROOK);
        board_update(board, F8, NONE);
      } else if (src == E8 && dst == C8) {
        board_update(board, A8, BLACK_ROOK);
        board_update(board, D8, NONE);
      }
    }
    break;
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
  board->ep = EMPTY_BB;
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
  int piece = extract_piece(move);
  int src = extract_from(move), dst = extract_to(move),
      flag = extract_flags(move);
  int color = piece_color(piece), result = 0;
  int attacker = piece_type(extract_piece(move)),
      victim = piece_type(board->squares[dst]);

  result =
      (color == WHITE)
          ? square_values[piece][(dst)] - square_values[piece][(src)]
          : square_values[piece][flip(dst)] - square_values[piece][flip(src)];

  if (is_capture(board, move))
    result += MVV_LVA[victim][attacker] + 10000;

  if (IS_PROMO(flag))
    result += (color == WHITE) ? square_values[PROMO_PT(flag)][dst] -
                                     square_values[WHITE_PAWN][dst]
                               : square_values[PROMO_PT(flag)][flip(dst)] -
                                     square_values[BLACK_PAWN][flip(dst)];
  else if (IS_ENP(flag))
    result += MVV_LVA[PAWN][PAWN] + 10000;

  *score = result;
}

INLINE int move_estimated_value(ChessBoard *board, Move move) {
  int flag = extract_flags(move);
  int value = SEEPieceValues[piece_type(board->squares[extract_to(move)])];

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
  int src = extract_from(move), dst = extract_to(move);
  int flag = extract_flags(move);
  if (IS_PROMO(flag)) {
    sprintf(buffer, "%s%s%c", SQ_TO_COORD[flip_63(src)],
            SQ_TO_COORD[flip_63(dst)], PROMOTION_TO_CHAR[PROMO_PT(flag)]);
  } else {
    sprintf(buffer, "%s%s", SQ_TO_COORD[flip_63(src)],
            SQ_TO_COORD[flip_63(dst)]);
  }
  return buffer;
}