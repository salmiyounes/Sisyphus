#include "gen.h"

// Helper functions

INLINE void emit_move_with_empty_flag(Move **moves, enum Square from,
                                      enum Square to, enum ColoredPiece piece) {
  emit_move(moves, from, to, piece, EMPTY_FLAG);
}

INLINE void emit_move_with_color(Move **moves, enum Square from, enum Square to,
                                 enum PieceType piece, enum Color color) {
  emit_move_with_empty_flag(moves, from, to, make_piece_type(piece, color));
}

INLINE void emit_promotion(Move **moves, enum Square from, enum Square to,
                           enum ColoredPiece piece, int flag) {
  emit_move(moves, from, to, piece, flag);
}

INLINE void emit_promotions(Move **moves, enum Square from, enum Square to,
                            enum ColoredPiece piece) {
  for (int flag = KNIGHT_PROMO_FLAG; flag <= QUEEN_PROMO_FLAG; flag++) {
    emit_promotion(moves, from, to, piece, flag);
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
    pop_lsb(src, srcs);
    bb dsts = BB_KNIGHT[src] & mask;
    while (dsts) {
      pop_lsb(dst, dsts);
      emit_move_with_color(&moves, src, dst, KNIGHT, color);
    }
  }
  return moves - ptr;
}

INLINE int gen_bishop_moves(Move *moves, bb srcs, bb mask, bb all, int color) {
  Move *ptr = moves;
  int src, dst;
  while (srcs) {
    pop_lsb(src, srcs);
    bb dsts = bb_bishop(src, all) & mask;
    while (dsts) {
      pop_lsb(dst, dsts);
      emit_move_with_color(&moves, src, dst, BISHOP, color);
    }
  }
  return moves - ptr;
}

INLINE int gen_rook_moves(Move *moves, bb srcs, bb mask, bb all, int color) {
  Move *ptr = moves;
  int src, dst;
  while (srcs) {
    pop_lsb(src, srcs);
    bb dsts = bb_rook(src, all) & mask;
    while (dsts) {
      pop_lsb(dst, dsts);
      emit_move_with_color(&moves, src, dst, ROOK, color);
    }
  }
  return moves - ptr;
}

INLINE int gen_queen_moves(Move *moves, bb srcs, bb mask, bb all, int color) {
  Move *ptr = moves;
  int src, dst;
  while (srcs) {
    pop_lsb(src, srcs);
    bb dsts = bb_queen(src, all) & mask;
    while (dsts) {
      pop_lsb(dst, dsts);
      emit_move_with_color(&moves, src, dst, QUEEN, color);
    }
  }
  return moves - ptr;
}

INLINE int gen_king_moves(Move *moves, bb srcs, bb mask, int color) {
  Move *ptr = moves;
  int src, dst;
  while (srcs) {
    pop_lsb(src, srcs);
    bb dsts = BB_KING[src] & mask;
    while (dsts) {
      pop_lsb(dst, dsts);
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
    pop_lsb(sq, p1);
    if (test_bit(promo, sq)) {
      emit_promotions(&moves, sq - 8, sq, WHITE_PAWN);
    } else {
      emit_move_with_empty_flag(&moves, sq - 8, sq, WHITE_PAWN);
    }
  }

  while (p2) {
    pop_lsb(sq, p2);
    emit_move_with_empty_flag(&moves, sq - 16, sq, WHITE_PAWN);
  }

  while (a1) {
    pop_lsb(sq, a1);
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
    pop_lsb(sq, a2);
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
    bb mask = BIT(F1) | BIT(G1);
    if (!(occ & mask)) {
      if (!(is_square_attacked_by(board, E1, BLACK)) &&
          !(is_square_attacked_by(board, F1, BLACK))) {
        emit_castle(&moves, E1, G1, WHITE_KING);
      }
    }
  }

  if (board->castle & CASTLE_WHITE_QUEEN_SIDE) {
    bb mask = BIT(B1) | BIT(C1) | BIT(D1);
    if (!(occ & mask)) {
      if (!(is_square_attacked_by(board, D1, BLACK)) &&
          !(is_square_attacked_by(board, E1, BLACK))) {
        emit_castle(&moves, E1, C1, WHITE_KING);
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
    pop_lsb(sq, a1);
    emit_move_with_empty_flag(&moves, sq - 7, sq, WHITE_PAWN);
  }

  while (a2) {
    pop_lsb(sq, a2);
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
    pop_lsb(sq, p1);
    if (test_bit(promo, sq)) {
      emit_promotions(&moves, sq + 8, sq, BLACK_PAWN);
    } else {
      emit_move_with_empty_flag(&moves, sq + 8, sq, BLACK_PAWN);
    }
  }
  while (p2) {
    pop_lsb(sq, p2);
    emit_move_with_empty_flag(&moves, sq + 16, sq, BLACK_PAWN);
  }
  while (a1) {
    pop_lsb(sq, a1);
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
    pop_lsb(sq, a2);
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
    bb mask = BIT(F8) | BIT(G8);
    if (!(occ & mask)) {
      if (!(is_square_attacked_by(board, E8, WHITE)) &&
          !(is_square_attacked_by(board, F8, WHITE))) {
        emit_castle(&moves, E8, G8, BLACK_KING);
      }
    }
  }

  if (board->castle & CASTLE_BLACK_QUEEN_SIDE) {
    bb mask = BIT(B8) | BIT(C8) | BIT(D8);
    if (!(occ & mask)) {
      if (!(is_square_attacked_by(board, D8, WHITE)) &&
          !(is_square_attacked_by(board, E8, WHITE))) {
        emit_castle(&moves, E8, C8, BLACK_KING);
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
    pop_lsb(sq, a1);
    emit_move_with_empty_flag(&moves, sq + 7, sq, BLACK_PAWN);
  }

  while (a2) {
    pop_lsb(sq, a2);
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

INLINE int gen_black_king_attacks_against(ChessBoard *board, Move *moves,
                                          bb mask) {
  return gen_king_moves(moves, board->bb_squares[BLACK_KING], mask, BLACK);
}

INLINE int gen_black_attacks_against(ChessBoard *board, Move *moves, bb mask) {
  static const AttacksGen black_attack_generators[] = {
      gen_black_pawn_attacks_against,   gen_black_knight_attacks_against,
      gen_black_bishop_attacks_against, gen_black_rook_attacks_against,
      gen_black_queen_attacks_against,  gen_black_king_attacks_against};

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
  return bb_attacks_to_king_square(board, kings(board) & them(board));
}

INLINE int is_check(ChessBoard *board) {
  bb occ = occ(board, BOTH);
  bb us = us(board), them = them(board);
  bb king = kings(board) & us;
  unsigned sq = lsb(king);
  bb ratt = bb_get_rook_attacks(sq, occ);
  bb batt = bb_get_bishop_attacks(sq, occ);
  if (ratt & (rooks(board) & them))
    return true;
  if (batt & (bishops(board) & them))
    return true;
  if ((ratt | batt) & (queens(board) & them))
    return true;
  if (bb_get_knight_attacks(sq) & (knights(board) & them))
    return true;
  if (bb_pawns_attacks(sq, board->color) & (pawns(board) & them))
    return true;
  return false;
}

bool is_mate(const ChessBoard *board0) {
  if (!is_check(board0))
    return false;

  Move moves[MAX_MOVES];
  int count = gen_moves(board0, moves);

  for (int i = 0; i < count; i++) {
    ChessBoard board1;
    board_copy(&board1, board0);
    if (make_move(&board1, moves[i]))
      return false;
  }
  return true;
}

bool is_valid(ChessBoard *board) {
  if (popcount(kings(board)) != 2)
    return false;
  if (popcount(kings(board) & white(board)) != 1)
    return false;
  if (popcount(kings(board) & black(board)) != 1)
    return false;
  if ((white(board) & black(board)) != 0)
    return false;
  if ((kings(board) & queens(board)) != 0)
    return false;
  if ((kings(board) & rooks(board)) != 0)
    return false;
  if ((kings(board) & bishops(board)) != 0)
    return false;
  if ((kings(board) & knights(board)) != 0)
    return false;
  if ((kings(board) & pawns(board)) != 0)
    return false;
  if ((queens(board) & rooks(board)) != 0)
    return false;
  if ((queens(board) & bishops(board)) != 0)
    return false;
  if ((queens(board) & knights(board)) != 0)
    return false;
  if ((queens(board) & pawns(board)) != 0)
    return false;
  if ((rooks(board) & bishops(board)) != 0)
    return false;
  if ((rooks(board) & knights(board)) != 0)
    return false;
  if ((rooks(board) & pawns(board)) != 0)
    return false;
  if ((bishops(board) & knights(board)) != 0)
    return false;
  if ((bishops(board) & pawns(board)) != 0)
    return false;
  if ((knights(board) & pawns(board)) != 0)
    return false;
  if (pawns(board) & BOARD_FILE_EDGE)
    return false;
  if ((white(board) | black(board)) !=
      (kings(board) | queens(board) | rooks(board) | bishops(board) |
       knights(board) | pawns(board)))
    return false;
  return !illegal_to_move(board);
}

bool move_gives_check(const ChessBoard *board0, const Move move) {
  ChessBoard board1;
  board_copy(&board1, board0);

  if (!make_move(&board1, move))
    return false; /* illegal move: it gives no check */
  return is_check(&board1);
}

bool move_gives_checkmate(const ChessBoard *board0, const Move move) {
  ChessBoard board1;
  board_copy(&board1, board0);

  if (!make_move(&board1, move))
    return false; /* illegal move: it gives no check */
  return is_mate(&board1);
}