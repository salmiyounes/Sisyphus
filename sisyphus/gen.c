#include "gen.h"

// Helper functions
INLINE void emit_move(Move **moves, enum Square from, enum Square to,
                      enum ColoredPiece piece, int flag) {
  *(*moves)++ = ENCODE_MOVE(from, to, piece, flag);
}

INLINE void emit_move_with_empty_flag(Move **moves, enum Square from,
                                      enum Square to, enum ColoredPiece piece) {
  emit_move(moves, from, to, piece, EMPTY_FLAG);
}

INLINE void emit_move_with_color(Move **moves, enum Square from, enum Square to,
                                 enum PieceType piece, enum Color color) {
  emit_move(moves, from, to, make_piece_type(piece, color), EMPTY_FLAG);
}

INLINE void emit_promotion(Move **moves, enum Square from, enum Square to,
                           enum ColoredPiece piece, int flag) {
  emit_move(moves, from, to, piece, flag);
}

INLINE void emit_promotions(Move **moves, enum Square from, enum Square to,
                            enum ColoredPiece piece) {
  for (int flag = KNIGHT_PROMO_FLAG; flag <= QUEEN_PROMO_FLAG;
       emit_promotion(moves, from, to, piece, flag), flag++) {
  }
}

INLINE void emit_en_passant(Move **moves, enum Square from, enum Square to,
                            enum ColoredPiece piece) {
  emit_move(moves, from, to, piece, ENP_FLAG);
}

INLINE void emit_castle(Move **moves, enum Square from, enum Square to,
                        enum ColoredPiece piece) {
  emit_move(moves, from, to, piece, CASTLE_FLAG);
}

INLINE int gen_knight_moves(Move *moves, bb srcs, bb mask, enum Color color) {
  Move *ptr = moves;
  int src, dst;
  while (srcs) {
    POP_LSB(src, srcs);
    bb dsts = BB_KNIGHT[src] & mask;
    while (dsts) {
      POP_LSB(dst, dsts);
      emit_move_with_color(&moves, src, dst, KNIGHT, color);
    }
  }
  return moves - ptr;
}

INLINE int gen_bishop_moves(Move *moves, bb srcs, bb mask, bb all, int color) {
  Move *ptr = moves;
  int src, dst;
  while (srcs) {
    POP_LSB(src, srcs);
    bb dsts = bb_bishop(src, all) & mask;
    while (dsts) {
      POP_LSB(dst, dsts);
      emit_move_with_color(&moves, src, dst, BISHOP, color);
    }
  }
  return moves - ptr;
}

INLINE int gen_rook_moves(Move *moves, bb srcs, bb mask, bb all, int color) {
  Move *ptr = moves;
  int src, dst;
  while (srcs) {
    POP_LSB(src, srcs);
    bb dsts = bb_rook(src, all) & mask;
    while (dsts) {
      POP_LSB(dst, dsts);
      emit_move_with_color(&moves, src, dst, ROOK, color);
    }
  }
  return moves - ptr;
}

INLINE int gen_queen_moves(Move *moves, bb srcs, bb mask, bb all, int color) {
  Move *ptr = moves;
  int src, dst;
  while (srcs) {
    POP_LSB(src, srcs);
    bb dsts = bb_queen(src, all) & mask;
    while (dsts) {
      POP_LSB(dst, dsts);
      emit_move_with_color(&moves, src, dst, QUEEN, color);
    }
  }
  return moves - ptr;
}

INLINE int gen_king_moves(Move *moves, bb srcs, bb mask, int color) {
  Move *ptr = moves;
  int src, dst;
  while (srcs) {
    POP_LSB(src, srcs);
    bb dsts = BB_KING[src] & mask;
    while (dsts) {
      POP_LSB(dst, dsts);
      emit_move_with_color(&moves, src, dst, KING, color);
    }
  }
  return moves - ptr;
}

INLINE int gen_white_pawn_moves(ChessBoard *board, Move *moves) {
  Move *ptr = moves;
  bb pawns = board->bb_squares[WHITE_PAWN];
  bb mask = board->occ[BLACK] | board->ep;
  bb promo = 0xff00000000000000L;
  bb p1 = (pawns << 8) & ~board->occ[BOTH];
  bb p2 = ((p1 & 0x0000000000ff0000L) << 8) & ~board->occ[BOTH];
  bb a1 = ((pawns & 0xfefefefefefefefeL) << 7) & mask;
  bb a2 = ((pawns & 0x7f7f7f7f7f7f7f7fL) << 9) & mask;
  int sq;

  while (p1) {
    POP_LSB(sq, p1);
    if (test_bit(promo, sq)) {
      emit_promotions(&moves, sq - 8, sq, WHITE_PAWN);
    } else {
      emit_move_with_empty_flag(&moves, sq - 8, sq, WHITE_PAWN);
    }
  }

  while (p2) {
    POP_LSB(sq, p2);
    emit_move_with_empty_flag(&moves, sq - 16, sq, WHITE_PAWN);
  }

  while (a1) {
    POP_LSB(sq, a1);
    if (test_bit(promo, sq)) {
      emit_promotions(&moves, sq - 7, sq, WHITE_PAWN);
    } else {
      if (test_bit(board->ep, sq)) {
        emit_en_passant(&moves, sq - 7, sq, WHITE_PAWN);
      } else {
        emit_move_with_empty_flag(&moves, sq - 7, sq, WHITE_PAWN);
      }
    }
  }

  while (a2) {
    POP_LSB(sq, a2);
    if (test_bit(promo, sq)) {
      emit_promotions(&moves, sq - 9, sq, WHITE_PAWN);
    } else {
      if (test_bit(board->ep, sq)) {
        emit_en_passant(&moves, sq - 9, sq, WHITE_PAWN);
      } else {
        emit_move_with_empty_flag(&moves, sq - 9, sq, WHITE_PAWN);
      }
    }
  }
  return moves - ptr;
}

INLINE int gen_white_knight_moves(ChessBoard *board, Move *moves) {
  return gen_knight_moves(moves, board->bb_squares[WHITE_KNIGHT],
                          ~board->occ[WHITE], WHITE);
}

INLINE int gen_white_bishop_moves(ChessBoard *board, Move *moves) {
  return gen_bishop_moves(moves, board->bb_squares[WHITE_BISHOP],
                          ~board->occ[WHITE], board->occ[BOTH], WHITE);
}

INLINE int gen_white_rook_moves(ChessBoard *board, Move *moves) {
  return gen_rook_moves(moves, board->bb_squares[WHITE_ROOK],
                        ~board->occ[WHITE], board->occ[BOTH], WHITE);
}

INLINE int gen_white_queen_moves(ChessBoard *board, Move *moves) {
  return gen_queen_moves(moves, board->bb_squares[WHITE_QUEEN],
                         ~board->occ[WHITE], board->occ[BOTH], WHITE);
}

INLINE int gen_white_king_moves(ChessBoard *board, Move *moves) {
  return gen_king_moves(moves, board->bb_squares[WHITE_KING],
                        ~board->occ[WHITE], WHITE);
}

INLINE int gen_white_king_castle(ChessBoard *board, Move *moves) {
  Move *ptr = moves;
  bb occ = board->occ[BOTH];

  if (board->castle & CASTLE_WHITE_KING_SIDE) {
    bb mask = BIT(E1) | BIT(F1);
    if (!(occ & mask)) {
      if (!(is_square_attacked_by(board, D1, BLACK)) &&
          !(is_square_attacked_by(board, E1, BLACK))) {
        emit_castle(&moves, D1, F1, WHITE_KING);
      }
    }
  }

  if (board->castle & CASTLE_WHITE_QUEEN_SIDE) {
    bb mask = BIT(A1) | BIT(B1) | BIT(C1);
    if (!(occ & mask)) {
      if (!(is_square_attacked_by(board, C1, BLACK)) &&
          !(is_square_attacked_by(board, D1, BLACK))) {
        emit_castle(&moves, D1, B1, WHITE_KING);
      }
    }
  }
  return moves - ptr;
}

INLINE int gen_white_moves(ChessBoard *board, Move *moves) {
  static const MoveGen white_generators[] = {
      gen_white_pawn_moves, gen_white_knight_moves, gen_white_bishop_moves,
      gen_white_rook_moves, gen_white_queen_moves,  gen_white_king_moves,
      gen_white_king_castle};

  Move *ptr = moves;
  for (size_t i = 0; i < ARRAY_SIZE(white_generators); i++) {
    moves += white_generators[i](board, moves);
  }
  return moves - ptr;
}

INLINE int gen_white_pawn_attacks_against(ChessBoard *board, Move *moves,
                                          bb mask) {
  Move *ptr = moves;
  bb pawns = board->bb_squares[WHITE_PAWN];
  bb a1 = ((pawns & 0xfefefefefefefefeL) << 7) & mask;
  bb a2 = ((pawns & 0x7f7f7f7f7f7f7f7fL) << 9) & mask;
  int sq;

  while (a1) {
    POP_LSB(sq, a1);
    emit_move_with_empty_flag(&moves, sq - 7, sq, WHITE_PAWN);
  }

  while (a2) {
    POP_LSB(sq, a2);
    emit_move_with_empty_flag(&moves, sq - 9, sq, WHITE_PAWN);
  }
  return moves - ptr;
}

INLINE int gen_white_knight_attacks_against(ChessBoard *board, Move *moves,
                                            bb mask) {
  return gen_knight_moves(moves, board->bb_squares[WHITE_KNIGHT], mask, WHITE);
}

INLINE int gen_white_bishop_attacks_against(ChessBoard *board, Move *moves,
                                            bb mask) {
  return gen_bishop_moves(moves, board->bb_squares[WHITE_BISHOP], mask,
                          board->occ[BOTH], WHITE);
}

INLINE int gen_white_rook_attacks_against(ChessBoard *board, Move *moves,
                                          bb mask) {
  return gen_rook_moves(moves, board->bb_squares[WHITE_ROOK], mask,
                        board->occ[BOTH], WHITE);
}

INLINE int gen_white_queen_attacks_against(ChessBoard *board, Move *moves,
                                           bb mask) {
  return gen_queen_moves(moves, board->bb_squares[WHITE_QUEEN], mask,
                         board->occ[BOTH], WHITE);
}

INLINE int gen_white_king_attacks_against(ChessBoard *board, Move *moves,
                                          bb mask) {
  return gen_king_moves(moves, board->bb_squares[WHITE_KING], mask, WHITE);
}

INLINE int gen_white_attacks_against(ChessBoard *board, Move *moves, bb mask) {
  static const AttacksGen white_attack_generators[] = {
      gen_white_pawn_attacks_against,   gen_white_knight_attacks_against,
      gen_white_bishop_attacks_against, gen_white_rook_attacks_against,
      gen_white_queen_attacks_against,  gen_white_king_attacks_against};

  Move *ptr = moves;
  for (size_t i = 0; i < ARRAY_SIZE(white_attack_generators); i++) {
    moves += white_attack_generators[i](board, moves, mask);
  }
  return moves - ptr;
}

INLINE int gen_white_checks(ChessBoard *board, Move *moves) {
  return gen_white_attacks_against(board, moves, board->bb_squares[BLACK_KING]);
}

INLINE int gen_black_pawn_moves(ChessBoard *board, Move *moves) {
  Move *ptr = moves;
  bb pawns = board->bb_squares[BLACK_PAWN];
  bb mask = board->occ[WHITE] | board->ep;

  bb promo = 0x00000000000000ffL;
  bb p1 = (pawns >> 8) & ~board->occ[BOTH];
  bb p2 = ((p1 & 0x0000ff0000000000L) >> 8) & ~board->occ[BOTH];
  bb a1 = ((pawns & 0x7f7f7f7f7f7f7f7fL) >> 7) & mask;
  bb a2 = ((pawns & 0xfefefefefefefefeL) >> 9) & mask;
  int sq;
  while (p1) {
    POP_LSB(sq, p1);
    if (test_bit(promo, sq)) {
      emit_promotions(&moves, sq + 8, sq, BLACK_PAWN);
    } else {
      emit_move_with_empty_flag(&moves, sq + 8, sq, BLACK_PAWN);
    }
  }
  while (p2) {
    POP_LSB(sq, p2);
    emit_move_with_empty_flag(&moves, sq + 16, sq, BLACK_PAWN);
  }
  while (a1) {
    POP_LSB(sq, a1);
    if (test_bit(promo, sq)) {
      emit_promotions(&moves, sq + 7, sq, BLACK_PAWN);
    } else {
      if (test_bit(board->ep, sq)) {
        emit_en_passant(&moves, sq + 7, sq, BLACK_PAWN);
      } else {
        emit_move_with_empty_flag(&moves, sq + 7, sq, BLACK_PAWN);
      }
    }
  }
  while (a2) {
    POP_LSB(sq, a2);
    if (test_bit(promo, sq)) {
      emit_promotions(&moves, sq + 9, sq, BLACK_PAWN);
    } else {
      if (test_bit(board->ep, sq)) {
        emit_en_passant(&moves, sq + 9, sq, BLACK_PAWN);
      } else {
        emit_move_with_empty_flag(&moves, sq + 9, sq, BLACK_PAWN);
      }
    }
  }
  return moves - ptr;
}

INLINE int gen_black_knight_moves(ChessBoard *board, Move *moves) {
  return gen_knight_moves(moves, board->bb_squares[BLACK_KNIGHT],
                          ~board->occ[BLACK], BLACK);
}

INLINE int gen_black_bishop_moves(ChessBoard *board, Move *moves) {
  return gen_bishop_moves(moves, board->bb_squares[BLACK_BISHOP],
                          ~board->occ[BLACK], board->occ[BOTH], BLACK);
}

INLINE int gen_black_rook_moves(ChessBoard *board, Move *moves) {
  return gen_rook_moves(moves, board->bb_squares[BLACK_ROOK],
                        ~board->occ[BLACK], board->occ[BOTH], BLACK);
}

INLINE int gen_black_queen_moves(ChessBoard *board, Move *moves) {
  return gen_queen_moves(moves, board->bb_squares[BLACK_QUEEN],
                         ~board->occ[BLACK], board->occ[BOTH], BLACK);
}

INLINE int gen_black_king_moves(ChessBoard *board, Move *moves) {
  return gen_king_moves(moves, board->bb_squares[BLACK_KING],
                        ~board->occ[BLACK], BLACK);
}

INLINE int gen_black_king_castle(ChessBoard *board, Move *moves) {
  Move *ptr = moves;
  bb occ = board->occ[BOTH];

  if (board->castle & CASTLE_BLACK_KING_SIDE) {
    bb mask = BIT(E8) | BIT(F8);
    if (!(occ & mask)) {
      if (!(is_square_attacked_by(board, D8, WHITE)) &&
          !(is_square_attacked_by(board, E8, WHITE))) {
        emit_castle(&moves, D8, F8, BLACK_KING);
      }
    }
  }

  if (board->castle & CASTLE_BLACK_QUEEN_SIDE) {
    bb mask = BIT(A8) | BIT(B8) | BIT(C8);
    if (!(occ & mask)) {
      if (!(is_square_attacked_by(board, C8, WHITE)) &&
          !(is_square_attacked_by(board, D8, WHITE))) {
        emit_castle(&moves, D8, B8, BLACK_KING);
      }
    }
  }
  return moves - ptr;
}

INLINE int gen_black_moves(ChessBoard *board, Move *moves) {
  static const MoveGen black_generators[] = {
      gen_black_pawn_moves, gen_black_knight_moves, gen_black_bishop_moves,
      gen_black_rook_moves, gen_black_queen_moves,  gen_black_king_moves,
      gen_black_king_castle};

  Move *ptr = moves;
  for (size_t i = 0; i < ARRAY_SIZE(black_generators); i++) {
    moves += black_generators[i](board, moves);
  }
  return moves - ptr;
}

INLINE int gen_black_pawn_attacks_against(ChessBoard *board, Move *moves,
                                          bb mask) {
  Move *ptr = moves;
  bb pawns = board->bb_squares[BLACK_PAWN];
  bb a1 = ((pawns & 0x7f7f7f7f7f7f7f7fL) >> 7) & mask;
  bb a2 = ((pawns & 0xfefefefefefefefeL) >> 9) & mask;
  int sq;

  while (a1) {
    POP_LSB(sq, a1);
    emit_move_with_empty_flag(&moves, sq + 7, sq, BLACK_PAWN);
  }

  while (a2) {
    POP_LSB(sq, a2);
    emit_move_with_empty_flag(&moves, sq + 9, sq, BLACK_PAWN);
  }
  return moves - ptr;
}

INLINE int gen_black_knight_attacks_against(ChessBoard *board, Move *moves,
                                            bb mask) {
  return gen_knight_moves(moves, board->bb_squares[BLACK_KNIGHT], mask, BLACK);
}

INLINE int gen_black_bishop_attacks_against(ChessBoard *board, Move *moves,
                                            bb mask) {
  return gen_bishop_moves(moves, board->bb_squares[BLACK_BISHOP], mask,
                          board->occ[BOTH], BLACK);
}

INLINE int gen_black_rook_attacks_against(ChessBoard *board, Move *moves,
                                          bb mask) {
  return gen_rook_moves(moves, board->bb_squares[BLACK_ROOK], mask,
                        board->occ[BOTH], BLACK);
}

INLINE int gen_black_queen_attacks_against(ChessBoard *board, Move *moves,
                                           bb mask) {
  return gen_queen_moves(moves, board->bb_squares[BLACK_QUEEN], mask,
                         board->occ[BOTH], BLACK);
}

INLINE int gen_black_king_attacks_agianst(ChessBoard *board, Move *moves,
                                          bb mask) {
  return gen_king_moves(moves, board->bb_squares[BLACK_KING], mask, BLACK);
}

INLINE int gen_black_attacks_against(ChessBoard *board, Move *moves, bb mask) {
  static const AttacksGen black_attack_generators[] = {
      gen_black_pawn_attacks_against,   gen_black_knight_attacks_against,
      gen_black_bishop_attacks_against, gen_black_rook_attacks_against,
      gen_black_queen_attacks_against,  gen_black_king_attacks_agianst};

  Move *ptr = moves;
  for (size_t i = 0; i < ARRAY_SIZE(black_attack_generators); i++) {
    moves += black_attack_generators[i](board, moves, mask);
  }
  return moves - ptr;
}

INLINE int gen_black_checks(ChessBoard *board, Move *moves) {
  return gen_black_attacks_against(board, moves, board->bb_squares[WHITE_KING]);
}

INLINE int gen_moves(ChessBoard *board, Move *moves) {
  return board->color ? gen_black_moves(board, moves)
                      : gen_white_moves(board, moves);
}

INLINE int gen_attacks(ChessBoard *board, Move *moves) {
  return board->color
             ? gen_black_attacks_against(board, moves, board->occ[WHITE])
             : gen_white_attacks_against(board, moves, board->occ[BLACK]);
}

INLINE int gen_legal_moves(ChessBoard *board, Move *moves) {
  Move temp[MAX_MOVES];
  Undo undo;
  int count = gen_moves(board, temp), size = 0;
  for (int i = 0; i < count; i++) {
    Move move = temp[i];
    do_move(board, move, &undo);
    if (!illegal_to_move(board))
      moves[size++] = move;
    undo_move(board, move, &undo);
  }
  return size;
}

INLINE int illegal_to_move(ChessBoard *board) {
  bb bb_king = board->bb_squares[board->color ? WHITE_KING : BLACK_KING];
  return bb_attacks_to_king_square(board, bb_king);
}

INLINE int is_check(ChessBoard *board) { return bb_is_check(board); }

int move_gives_check(ChessBoard *board, const Move move) {
  Undo undo;

  do_move(board, move, &undo);
  int flag = is_check(board);
  undo_move(board, move, &undo);

  return flag;
}