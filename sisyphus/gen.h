#ifndef GEN_H
#define GEN_H

#include "bb.h"
#include "board.h"
#include "move.h"

/* Macros */
#define EMIT_MOVE(m, from, to, piece)                                          \
  *(m++) = ENCODE_MOVE(from, to, piece, EMPTY_FLAG); // Emit normal move

#define EMIT_PROMOTION(m, from, to, piece, flag)                               \
  *(m++) = ENCODE_MOVE(from, to, piece, flag) // Emit promotion move

#define EMIT_PROMOTIONS(m, from, to, piece)                                    \
  for (int flag = KNIGHT_PROMO_FLAG; flag <= QUEEN_PROMO_FLAG;                 \
       EMIT_PROMOTION(m, from, to, piece, flag),                               \
           flag++) // Emit all promotion types

#define EMIT_EN_PASSANT(m, from, to, piece)                                    \
  *(m++) = ENCODE_MOVE(from, to, piece, ENP_FLAG); // Emit en passant move

#define EMIT_CASTLE(m, from, to, piece)                                        \
  *(m++) = ENCODE_MOVE(from, to, piece, CASTLE_FLAG); // Emit castling move

// White piece move generation
int gen_white_moves(ChessBoard *board, Move *moves); // Generate all white moves
int gen_white_pawn_moves(ChessBoard *board,
                         Move *moves); // Generate white pawn moves
int gen_white_knight_moves(ChessBoard *board,
                           Move *moves); // Generate white knight moves
int gen_white_bishop_moves(ChessBoard *board,
                           Move *moves); // Generate white bishop moves
int gen_white_rook_moves(ChessBoard *board,
                         Move *moves); // Generate white rook moves
int gen_white_queen_moves(ChessBoard *board,
                          Move *moves); // Generate white queen moves
int gen_white_king_moves(ChessBoard *board,
                         Move *moves); // Generate white king moves
int gen_white_attacks_against(ChessBoard *board, Move *moves,
                              bb mask); // Generate white attacks to squares

// Black piece move generation
int gen_black_moves(ChessBoard *board, Move *moves); // Generate all black moves
int gen_black_pawn_moves(ChessBoard *board,
                         Move *moves); // Generate black pawn moves
int gen_black_knight_moves(ChessBoard *board,
                           Move *moves); // Generate black knight moves
int gen_black_bishop_moves(ChessBoard *board,
                           Move *moves); // Generate black bishop moves
int gen_black_rook_moves(ChessBoard *board,
                         Move *moves); // Generate black rook moves
int gen_black_queen_moves(ChessBoard *board,
                          Move *moves); // Generate black queen moves
int gen_black_king_moves(ChessBoard *board,
                         Move *moves); // Generate black king moves
int gen_black_attacks_against(ChessBoard *board, Move *moves,
                              bb mask); // Generate black attacks to squares

// General move generation
int gen_knight_moves(Move *moves, bb srcs, bb mask,
                     int color); // Generate knight moves for given squares
int gen_attacks(ChessBoard *board, Move *moves); // Generate all attacking moves
int gen_legal_moves(ChessBoard *board, Move *moves); // Generate all legal moves
int gen_moves(ChessBoard *board, Move *moves); // Generate all possible moves

// Move validation and check detection
int illegal_to_move(ChessBoard *board); // Check if position is illegal
int is_check(ChessBoard *board);        // Check if king is in check
int move_gives_check(ChessBoard *board,
                     const Move move); // Check if move gives check

#endif // GEN_H