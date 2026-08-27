#include "perft.h"
#include "move.h"

#define SIZE (1 << 20)
#define MASK ((SIZE) - 1)

Entry_t TABLE[SIZE] = {0};

bb perft_test(ChessBoard *board, int depth) {
  Undo undo;
  Move moves[MAX_MOVES];
  bb nodes = U64(0);
  int count = 0;
  depth = MAX(depth, 0);

  if (!depth)
    return U64(1);

  Entry_t *entry = &TABLE[(board->hash & MASK)];

  if (entry->key == board->hash && entry->depth == depth) {
    return entry->value;
  }
  count = gen_moves(board, moves);
  for (count -= 1; count >= 0; count--) {
    Move move = moves[count];
    do_move(board, move, &undo);
    if (!illegal_to_move(board))
      nodes += perft_test(board, depth - 1);
    undo_move(board, move, &undo);
  }

  entry->depth = depth;
  entry->key = board->hash;
  entry->value = nodes;

  return nodes;
}
