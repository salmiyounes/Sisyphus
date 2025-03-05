#include "gen.h"

int gen_knight_moves(Move *moves, bb srcs, bb mask, int color) {
    Move *ptr = moves;
    int src, dst;
    while (srcs) {
        POP_LSB(src, srcs);
        bb dsts = BB_KNIGHT[src] & mask;
        while (dsts) {
            POP_LSB(dst, dsts);
            EMIT_MOVE(moves, src, dst, make_piece_type(KNIGHT, color), EMPTY_FLAG);
        }
    }

    return moves - ptr;
}

int gen_bishop_moves(Move *moves, bb srcs, bb mask, bb all, int color) {
    Move *ptr = moves;
    int src, dst;
    while (srcs) {
        POP_LSB(src, srcs);
        bb dsts = bb_bishop(src, all) & mask;
        while (dsts) {
            POP_LSB(dst, dsts);
            EMIT_MOVE(moves, src, dst, make_piece_type(BISHOP, color), EMPTY_FLAG);
        }
    }
    return moves - ptr;
}
  
int gen_rook_moves(Move *moves, bb srcs, bb mask, bb all, int color) {
    Move *ptr = moves;
    int src, dst;
    while (srcs) {
        POP_LSB(src, srcs);
        bb dsts = bb_rook(src, all) & mask;
        while (dsts) {
            POP_LSB(dst, dsts);
            EMIT_MOVE(moves, src, dst, make_piece_type(ROOK, color), EMPTY_FLAG);
        }
    }
    return moves - ptr;
}

int gen_queen_moves(Move *moves, bb srcs, bb mask, bb all, int color) {
    Move *ptr = moves;
    int src, dst;
    while (srcs) {
        POP_LSB(src, srcs);
        bb dsts = bb_queen(src, all) & mask;
        while (dsts) {
            POP_LSB(dst, dsts);
            EMIT_MOVE(moves, src, dst, make_piece_type(QUEEN, color), EMPTY_FLAG);
        }
    }
    return moves - ptr;
}

int gen_king_moves(Move *moves, bb srcs, bb mask, int color) {
    Move *ptr = moves;
    int src, dst;
    while (srcs) {
        POP_LSB(src, srcs);
        bb dsts = BB_KING[src] & mask;
        while (dsts) {
            POP_LSB(dst, dsts);
            EMIT_MOVE(moves, src, dst, make_piece_type(KING, color), EMPTY_FLAG);
        }
    }
    return moves - ptr;
}

int gen_white_pawn_moves(ChessBoard *board, Move *moves) {
    Move *ptr = moves;
    bb pawns  = board->bb_squares[WHITE_PAWN];
    bb mask   = board->occ[BLACK] | board->ep;
    bb promo  = 0xff00000000000000L;
    bb p1     = (pawns << 8) & ~board->occ[BOTH];
    bb p2     = ((p1 & 0x0000000000ff0000L) << 8) & ~board->occ[BOTH];
    bb a1     = ((pawns & 0xfefefefefefefefeL) << 7) & mask;
    bb a2     = ((pawns & 0x7f7f7f7f7f7f7f7fL) << 9) & mask;
    int sq;

    while (p1) {
        POP_LSB(sq, p1);
        if (test_bit(promo, sq)) {
            EMIT_PROMOTIONS(moves, sq - 8, sq, WHITE_PAWN);
        } else {
            EMIT_MOVE(moves, sq - 8, sq, WHITE_PAWN, EMPTY_FLAG);
        }
    }

    while (p2) {
        POP_LSB(sq, p2);
        EMIT_MOVE(moves, sq - 16, sq, WHITE_PAWN, EMPTY_FLAG);
    }

    while (a1) {
        POP_LSB(sq, a1);
        if (test_bit(promo, sq)) {
            EMIT_PROMOTIONS(moves, sq - 7, sq, WHITE_PAWN);
        }
         else {
            if (test_bit(board->ep, sq)) {
                EMIT_EN_PASSANT(moves, sq - 7, sq, WHITE_PAWN);
            } else {
                EMIT_MOVE(moves, sq - 7, sq, WHITE_PAWN, EMPTY_FLAG);
            }
        }
    }

    while (a2) {
        POP_LSB(sq, a2);
        if (test_bit(promo, sq)) {
            EMIT_PROMOTIONS(moves, sq - 9, sq, WHITE_PAWN);
        }
        else {
            if (test_bit(board->ep, sq)) {
                EMIT_EN_PASSANT(moves, sq - 9, sq, WHITE_PAWN);
            } else {
                EMIT_MOVE(moves, sq - 9, sq, WHITE_PAWN, EMPTY_FLAG);
            }
        }
    }

    return moves - ptr;
}

INLINE int gen_white_knight_moves(ChessBoard *board, Move *moves) {
    return gen_knight_moves(
        moves, board->bb_squares[WHITE_KNIGHT], ~board->occ[WHITE], WHITE);
}

INLINE int gen_white_bishop_moves(ChessBoard *board, Move *moves) {
    return gen_bishop_moves(
        moves, board->bb_squares[WHITE_BISHOP], ~board->occ[WHITE], board->occ[BOTH], WHITE);
}

INLINE int gen_white_rook_moves(ChessBoard *board, Move *moves) {
    return gen_rook_moves(
        moves, board->bb_squares[WHITE_ROOK], ~board->occ[WHITE], board->occ[BOTH], WHITE);
}

INLINE int gen_white_queen_moves(ChessBoard *board, Move *moves) {
    return gen_queen_moves(
        moves, board->bb_squares[WHITE_QUEEN], ~board->occ[WHITE], board->occ[BOTH], WHITE);
}

INLINE int gen_white_king_moves(ChessBoard *board, Move *moves) {
    return gen_king_moves(
        moves, board->bb_squares[WHITE_KING], ~board->occ[WHITE], WHITE);
}

int gen_white_king_castle(ChessBoard *board, Move *moves) {
    Move *ptr = moves;
    if (board->castle & CASTLE_WHITE_KING_SIDE) {
        if (!(board->occ[BOTH] & 0x0000000000000060L)) {
            Move dummy[MAX_MOVES];
            bb mask = 0x0000000000000030L;
            if (!(gen_black_attacks_against(board, dummy, mask))) {
                EMIT_CASTLE(moves, 4, 6, WHITE_KING);
            }
        }
    }
    if (board->castle & CASTLE_WHITE_QUEEN_SIDE) {
        if (!(board->occ[BOTH] & 0x000000000000000eL)) {
            Move dummy[MAX_MOVES];
            bb mask = 0x0000000000000018L;
            if (!(gen_black_attacks_against(board, dummy, mask))) {
                EMIT_CASTLE(moves, 4, 2, WHITE_KING);
            }
        }
    }
    return moves - ptr;
}

INLINE int gen_white_moves(ChessBoard *board, Move *moves) {
    Move *ptr = moves;
    moves += gen_white_pawn_moves(board, moves);
    moves += gen_white_knight_moves(board, moves);
    moves += gen_white_bishop_moves(board, moves);
    moves += gen_white_rook_moves(board, moves);
    moves += gen_white_queen_moves(board, moves);
    moves += gen_white_king_moves(board, moves);
    moves += gen_white_king_castle(board, moves);

    return moves - ptr;
}

int gen_white_pawn_attacks_against(ChessBoard *board, Move *moves, bb mask) {
    Move *ptr = moves;
    bb pawns  = board->bb_squares[WHITE_PAWN];
    bb a1     = ((pawns & 0xfefefefefefefefeL) << 7) & mask;
    bb a2     = ((pawns & 0x7f7f7f7f7f7f7f7fL) << 9) & mask;
    int sq;

    while (a1) {
        POP_LSB(sq, a1);
        EMIT_MOVE(moves, sq - 7, sq, WHITE_PAWN, EMPTY_FLAG);
    }

    while (a2) {
        POP_LSB(sq, a2);
        EMIT_MOVE(moves, sq - 9, sq, WHITE_PAWN, EMPTY_FLAG);
    }

    return moves - ptr;
}

INLINE int gen_white_knight_attacks_against(ChessBoard *board, Move *moves, bb mask) {
    return gen_knight_moves(
        moves, board->bb_squares[WHITE_KNIGHT], mask, WHITE);
}

INLINE int gen_white_bishop_attacks_against(ChessBoard *board, Move *moves, bb mask) {
    return gen_bishop_moves(
        moves, board->bb_squares[WHITE_BISHOP], mask, board->occ[BOTH], WHITE);
}

INLINE int gen_white_rook_attacks_against(ChessBoard *board, Move *moves, bb mask) {
    return gen_rook_moves(
        moves, board->bb_squares[WHITE_ROOK], mask, board->occ[BOTH], WHITE);
}

INLINE int gen_white_queen_attacks_against(ChessBoard *board, Move *moves, bb mask) {
    return gen_queen_moves(
        moves, board->bb_squares[WHITE_QUEEN], mask, board->occ[BOTH], WHITE);
}

INLINE int gen_white_king_attacks_against(ChessBoard *board, Move *moves, bb mask) {
    return gen_king_moves(
        moves, board->bb_squares[WHITE_KING], mask, WHITE);
}

INLINE int gen_white_attacks_against(ChessBoard *board, Move *moves, bb mask) {
    Move *ptr = moves;
    moves += gen_white_pawn_attacks_against(board, moves, mask);
    moves += gen_white_knight_attacks_against(board, moves, mask);
    moves += gen_white_bishop_attacks_against(board, moves, mask);
    moves += gen_white_rook_attacks_against(board, moves, mask);
    moves += gen_white_queen_attacks_against(board, moves, mask);
    moves += gen_white_king_attacks_against(board, moves, mask);

    return moves - ptr;
}

INLINE int gen_white_checks(ChessBoard *board, Move *moves) {
    return gen_white_attacks_against(board, moves, board->bb_squares[BLACK_KING]);
}

int gen_black_pawn_moves(ChessBoard *board, Move *moves) {
    Move *ptr = moves;
    bb pawns  = board->bb_squares[BLACK_PAWN];
    bb mask = board->occ[WHITE] | board->ep;

    bb promo  = 0x00000000000000ffL;
    bb p1     = (pawns >> 8) & ~board->occ[BOTH];
    bb p2     = ((p1 & 0x0000ff0000000000L) >> 8) & ~board->occ[BOTH];
    bb a1     = ((pawns & 0x7f7f7f7f7f7f7f7fL) >> 7) & mask; 
    bb a2     = ((pawns & 0xfefefefefefefefeL) >> 9) & mask;
    int sq;
    while (p1) {
        POP_LSB(sq, p1);
        if (test_bit(promo, sq)) {
            EMIT_PROMOTIONS(moves, sq + 8, sq, BLACK_PAWN);
        }
        else {
            EMIT_MOVE(moves, sq + 8, sq, BLACK_PAWN, EMPTY_FLAG);
        } 
    }
    while (p2) {
        POP_LSB(sq, p2);
        EMIT_MOVE(moves, sq + 16, sq, BLACK_PAWN, EMPTY_FLAG);
    }
    while (a1) {
        POP_LSB(sq, a1);
        if (test_bit(promo, sq)) {
            EMIT_PROMOTIONS(moves, sq + 7, sq, BLACK_PAWN);
        }
        else {
            if (test_bit(board->ep, sq)) {
                EMIT_EN_PASSANT(moves, sq + 7, sq, BLACK_PAWN);
            } else {
               EMIT_MOVE(moves, sq + 7, sq, BLACK_PAWN, EMPTY_FLAG);
            }
        } 
    }
    while (a2) {
        POP_LSB(sq, a2);
        if (test_bit(promo, sq)) {
            EMIT_PROMOTIONS(moves, sq + 9, sq, BLACK_PAWN);
        }
        else {
            if (test_bit(board->ep, sq)) {
                EMIT_EN_PASSANT(moves, sq + 9, sq, BLACK_PAWN);
            } else {
                EMIT_MOVE(moves, sq + 9, sq, BLACK_PAWN, EMPTY_FLAG);
            }
        }
    }

    return moves - ptr;

}

INLINE int gen_black_knight_moves(ChessBoard *board, Move *moves) {
    return gen_knight_moves(
        moves, board->bb_squares[BLACK_KNIGHT], ~board->occ[BLACK], BLACK);
}

INLINE int gen_black_bishop_moves(ChessBoard *board, Move *moves) {
    return gen_bishop_moves(
        moves, board->bb_squares[BLACK_BISHOP], ~board->occ[BLACK], board->occ[BOTH], BLACK);
}

INLINE int gen_black_rook_moves(ChessBoard *board, Move *moves) {
    return gen_rook_moves(
        moves, board->bb_squares[BLACK_ROOK], ~board->occ[BLACK], board->occ[BOTH], BLACK);
}

INLINE int gen_black_queen_moves(ChessBoard *board, Move *moves) {
    return gen_queen_moves(
        moves, board->bb_squares[BLACK_QUEEN], ~board->occ[BLACK], board->occ[BOTH], BLACK);
}

INLINE int gen_black_king_moves(ChessBoard *board, Move *moves) {
    return gen_king_moves(
        moves, board->bb_squares[BLACK_KING], ~board->occ[BLACK], BLACK);
}

int gen_black_king_castle(ChessBoard *board, Move *moves) {
    Move *ptr = moves;
    if (board->castle & CASTLE_BLACK_KING_SIDE) {
        if (!(board->occ[BOTH] & 0x6000000000000000L)) {
            Move dummy[MAX_MOVES];
            bb mask = 0x3000000000000000L;
            if (!(gen_white_attacks_against(board, dummy, mask))) {
                EMIT_CASTLE(moves, 60, 62, BLACK_KING);
            }
        }
    }
    if (board->castle & CASTLE_BLACK_QUEEN_SIDE) {
            if (!(board->occ[BOTH] & 0x0e00000000000000L)) {
                Move dummy[MAX_MOVES];
                bb mask =  0x1800000000000000L;
                if (!(gen_white_attacks_against(board, dummy, mask))) {
                    EMIT_CASTLE(moves, 60, 58, BLACK_KING);
                }           
        }
    }
    return moves - ptr;
}

INLINE int gen_black_moves(ChessBoard *board, Move *moves) {
    Move *ptr = moves;
    moves += gen_black_pawn_moves(board, moves);
    moves += gen_black_knight_moves(board, moves);
    moves += gen_black_bishop_moves(board, moves);
    moves += gen_black_rook_moves(board, moves);
    moves += gen_black_queen_moves(board, moves);
    moves += gen_black_king_moves(board, moves);
    moves += gen_black_king_castle(board, moves);

    return moves - ptr;
}

int gen_black_pawn_attacks_against(ChessBoard *board, Move *moves, bb mask) {
    Move *ptr = moves;
    bb pawns  = board->bb_squares[BLACK_PAWN];
    bb a1     = ((pawns & 0x7f7f7f7f7f7f7f7fL) >> 7) & mask;
    bb a2     = ((pawns & 0xfefefefefefefefeL) >> 9) & mask;
    int sq;

    while (a1) {
        POP_LSB(sq, a1);
        EMIT_MOVE(moves, sq + 7, sq, BLACK_PAWN, EMPTY_FLAG);
    }

    while (a2) {
        POP_LSB(sq, a2);
        EMIT_MOVE(moves, sq + 9, sq, BLACK_PAWN, EMPTY_FLAG);
    }

    return moves - ptr;
}

INLINE int gen_black_knight_attacks_against(ChessBoard *board, Move *moves, bb mask) {
    return gen_knight_moves(
        moves, board->bb_squares[BLACK_KNIGHT], mask, BLACK);
}

INLINE int gen_black_bishop_attacks_against(ChessBoard *board, Move *moves, bb mask) {
    return gen_bishop_moves(
        moves, board->bb_squares[BLACK_BISHOP], mask, board->occ[BOTH], BLACK);
}

INLINE int gen_black_rook_attacks_against(ChessBoard *board, Move *moves, bb mask) {
    return gen_rook_moves(
        moves, board->bb_squares[BLACK_ROOK], mask, board->occ[BOTH], BLACK);
}

INLINE int gen_black_queen_attacks_against(ChessBoard *board, Move *moves, bb mask) {
    return gen_queen_moves(
        moves, board->bb_squares[BLACK_QUEEN], mask, board->occ[BOTH], BLACK);
}

INLINE int gen_black_king_attacks_agianst(ChessBoard *board, Move *moves, bb mask) {
    return gen_king_moves(
        moves, board->bb_squares[BLACK_KING], mask, BLACK);
}

INLINE int gen_black_attacks_against(ChessBoard *board, Move *moves, bb mask) {
    Move *ptr = moves;
    moves += gen_black_pawn_attacks_against(board, moves, mask);
    moves += gen_black_knight_attacks_against(board, moves, mask);
    moves += gen_black_bishop_attacks_against(board, moves, mask);
    moves += gen_black_rook_attacks_against(board, moves, mask);
    moves += gen_black_queen_attacks_against(board, moves, mask);
    moves += gen_black_king_attacks_agianst(board, moves, mask);

    return moves - ptr;
}

INLINE int gen_black_checks(ChessBoard *board, Move *moves) {
    return gen_black_attacks_against(
        board, moves, board->bb_squares[WHITE_KING]);
}

INLINE int gen_moves(ChessBoard *board, Move *moves) {
    return board->color ? gen_black_moves(board, moves)
                        : gen_white_moves(board, moves);
}

INLINE int gen_attacks(ChessBoard *board, Move *moves) {
    return board->color ? gen_black_attacks_against(board, moves, board->occ[WHITE])
                        : gen_white_attacks_against(board, moves, board->occ[BLACK]);
}

INLINE int gen_legal_moves(ChessBoard *board, Move *moves) {
    Move temp[MAX_MOVES]; 
    Undo undo;
    int count  = gen_moves(board, temp), size = 0;
    for (int i = 0; i < count; i++) {
        Move move = temp[i];
        do_move(board, move, &undo);
        if (!illegal_to_move(board)) moves[size++] = move;
        undo_move(board, move, &undo);
    }
    return size;
}

INLINE int illegal_to_move(ChessBoard *board) {
    return board->color ? attacks_to_king_square(board, board->bb_squares[WHITE_KING])
                        : attacks_to_king_square(board, board->bb_squares[BLACK_KING]);
}

INLINE int is_check(ChessBoard *board) {
    Move moves[MAX_MOVES];
    return board->color ? gen_white_checks(board, moves)
                        : gen_black_checks(board, moves);
}

int move_gives_check(ChessBoard *board, const Move move) {
    Undo undo;
    
    do_move(board, move, &undo);
    int flag = is_check(board);
    undo_move(board, move, &undo);

    return flag;
}