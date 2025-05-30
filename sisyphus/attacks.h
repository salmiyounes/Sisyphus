#ifndef ATTACKS_H
#define ATTACKS_H

#include "bb.h"
#include "types.h"

bb get_bishop_attacks(int sq, bb obs);

bb get_rook_attacks(int sq, bb obs);

int attacks_to_king_square(ChessBoard *board, bb b_king);

bb attacks_to_square(ChessBoard *board, int sq, bb occ);

#endif // ATTACKS_H