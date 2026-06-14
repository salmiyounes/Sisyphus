#include "eval.h"

int eval(ChessBoard *board) { return pesto_eval(board); }

int pesto_eval(ChessBoard *board) {
  int side2move = board->color;
  int mgScore = board->mg[side2move] - board->mg[side2move ^ BLACK];
  int egScore = board->eg[side2move] - board->eg[side2move ^ BLACK];
  int mgPhase = board->gamePhase;
  if (mgPhase > 24)
    mgPhase = 24;
  int egPhase = 24 - mgPhase;
  return (mgScore * mgPhase + egScore * egPhase) / 24;
}

int count_stacked_pawns(bb pawns, int count) {
  int result = 0;
  result += popcount(pawns & FILE_A) == count;
  result += popcount(pawns & FILE_B) == count;
  result += popcount(pawns & FILE_C) == count;
  result += popcount(pawns & FILE_D) == count;
  result += popcount(pawns & FILE_E) == count;
  result += popcount(pawns & FILE_F) == count;
  result += popcount(pawns & FILE_G) == count;
  result += popcount(pawns & FILE_H) == count;
  return result;
}

int evaluate_white_pawns(ChessBoard *board) {
  bb pawns = board->bb_squares[WHITE_PAWN];
  int score = 0;
  score -= count_stacked_pawns(pawns, 2) * 50;
  score -= count_stacked_pawns(pawns, 3) * 100;
  return score;
}

int evaluate_black_pawns(ChessBoard *board) {
  bb pawns = board->bb_squares[BLACK_PAWN];
  int score = 0;
  score -= count_stacked_pawns(pawns, 2) * 50;
  score -= count_stacked_pawns(pawns, 3) * 100;
  return score;
}

int evaluate_pawns(ChessBoard *board) {
  int score = 0;
  score += evaluate_white_pawns(board);
  score -= evaluate_black_pawns(board);

  return (board->color == WHITE) ? score : -score;
}