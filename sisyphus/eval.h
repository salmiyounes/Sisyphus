#ifndef EVAL_H
#define EVAL_H

#include "types.h"

int pesto_eval(ChessBoard *board);

int evaluate_pawns(ChessBoard *board);

int eval(ChessBoard *board);

#endif // EVAL_H