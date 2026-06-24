#include "bb.h"

bb BB_PAWNS[2][64];
bb BB_KNIGHT[64];
bb BB_BISHOP[64];
bb BB_ROOK[64];
bb BB_KING[64];

const bb MAGIC_BISHOP[64] = {
    0x010a0a1023020080L, 0x0050100083024000L, 0x8826083200800802L,
    0x0102408100002400L, 0x0414242008000000L, 0x0414242008000000L,
    0x0804230108200880L, 0x0088840101012000L, 0x0400420202041100L,
    0x0400420202041100L, 0x1100300082084211L, 0x0000124081000000L,
    0x0405040308000411L, 0x01000110089c1008L, 0x0030108805101224L,
    0x0010808041101000L, 0x2410002102020800L, 0x0010202004098180L,
    0x1104000808001010L, 0x274802008a044000L, 0x1400884400a00000L,
    0x0082000048260804L, 0x4004840500882043L, 0x0081001040680440L,
    0x4282180040080888L, 0x0044200002080108L, 0x2404c80a04002400L,
    0x2020808028020002L, 0x0129010050304000L, 0x0008020108430092L,
    0x005600450c884800L, 0x005600450c884800L, 0x001004501c200301L,
    0xa408025880100100L, 0x1042080300060a00L, 0x4100a00801110050L,
    0x11240100c40c0040L, 0x24a0281141188040L, 0x08100c4081030880L,
    0x020c310201002088L, 0x006401884600c280L, 0x1204028210809888L,
    0x8000a01402005002L, 0x041d8a021a000400L, 0x041d8a021a000400L,
    0x000201a102004102L, 0x0408010842041282L, 0x000201a102004102L,
    0x0804230108200880L, 0x0804230108200880L, 0x8001010402090010L,
    0x0008000042020080L, 0x4200012002440000L, 0x80084010228880a0L,
    0x4244049014052040L, 0x0050100083024000L, 0x0088840101012000L,
    0x0010808041101000L, 0x1090c00110511001L, 0x2124000208420208L,
    0x0800102118030400L, 0x0010202120024080L, 0x00024a4208221410L,
    0x010a0a1023020080L};

const bb MAGIC_ROOK[64] = {
    0x0080004000608010L, 0x2240100040012002L, 0x008008a000841000L,
    0x0100204900500004L, 0x020008200200100cL, 0x40800c0080020003L,
    0x0080018002000100L, 0x4200042040820d04L, 0x10208008a8400480L,
    0x4064402010024000L, 0x2181002000c10212L, 0x5101000850002100L,
    0x0010800400080081L, 0x0012000200300815L, 0x060200080e002401L,
    0x4282000420944201L, 0x1040208000400091L, 0x0010004040002008L,
    0x0082020020804011L, 0x0005420010220208L, 0x8010510018010004L,
    0x05050100088a1400L, 0x0009008080020001L, 0x2001060000408c01L,
    0x0060400280008024L, 0x9810401180200382L, 0x0200201200420080L,
    0x0280300100210048L, 0x0000080080800400L, 0x0002010200081004L,
    0x8089000900040200L, 0x0040008200340047L, 0x0400884010800061L,
    0xc202401000402000L, 0x0800401301002004L, 0x4c43502042000a00L,
    0x0004a80082800400L, 0xd804040080800200L, 0x060200080e002401L,
    0x0203216082000104L, 0x0000804000308000L, 0x004008100020a000L,
    0x1001208042020012L, 0x0400220088420010L, 0x8010510018010004L,
    0x8009000214010048L, 0x6445006200130004L, 0x000a008402460003L,
    0x0080044014200240L, 0x0040012182411500L, 0x0003102001430100L,
    0x4c43502042000a00L, 0x1008000400288080L, 0x0806003008040200L,
    0x4200020801304400L, 0x8100640912804a00L, 0x300300a043168001L,
    0x0106610218400081L, 0x008200c008108022L, 0x0201041861017001L,
    0x00020010200884e2L, 0x0205000e18440001L, 0x202008104a08810cL,
    0x800a208440230402L};

const int SHIFT_BISHOP[64] = {
    58, 59, 59, 59, 59, 59, 59, 58, 59, 59, 59, 59, 59, 59, 59, 59,
    59, 59, 57, 57, 57, 57, 59, 59, 59, 59, 57, 55, 55, 57, 59, 59,
    59, 59, 57, 55, 55, 57, 59, 59, 59, 59, 57, 57, 57, 57, 59, 59,
    59, 59, 59, 59, 59, 59, 59, 59, 58, 59, 59, 59, 59, 59, 59, 58};

const int SHIFT_ROOK[64] = {
    52, 53, 53, 53, 53, 53, 53, 52, 53, 54, 54, 54, 54, 54, 54, 53,
    53, 54, 54, 54, 54, 54, 54, 53, 53, 54, 54, 54, 54, 54, 54, 53,
    53, 54, 54, 54, 54, 54, 54, 53, 53, 54, 54, 54, 54, 54, 54, 53,
    53, 54, 54, 54, 54, 54, 54, 53, 52, 53, 53, 53, 53, 53, 53, 52,
};

int OFFSET_BISHOP[64];
int OFFSET_ROOK[64];

bb ATTACK_BISHOP[5248];
bb ATTACK_ROOK[102400];

int get_lsb(bb bbit) {
  // https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html#index-_005f_005fbuiltin_005fclz
  // If bbit is 0, the result is undefined
  assert(bbit);
  return __builtin_ctzll(bbit);
}

int get_msb(bb bbit) {
  assert(bbit);
  return __builtin_ctzll(bbit) ^ 63;
}

int popcount(bb bbit) { return __builtin_popcountll(bbit); }

int several(bb bbit) { return bbit & (bbit - 1); }

bool test_bit(bb bbit, const int sq) {
  assert(sq >= 0 && sq < SQUARE_NB);
  return (bool)(bbit & BIT(sq));
}

int square(int rank, int file) {
  assert(0 <= rank && rank < RANK_NB);
  assert(0 <= file && file < FILE_NB);
  return rank * FILE_NB + file;
}

int file_of(int sq) {
  assert(0 <= sq && sq < SQUARE_NB);
  return sq % FILE_NB;
}

int rank_of(int sq) {
  assert(0 <= sq && sq < SQUARE_NB);
  return sq / RANK_NB;
}

int make_piece_type(int pc, int color) {
  assert(color == WHITE || color == BLACK);
  assert(pc >= PAWN && pc <= KING);
  return (pc << 1) + color;
}

int bb_squares(bb value, int squares[64]) {
  int i = 0;
  int sq;
  while (value) {
    POP_LSB(sq, value);
    squares[i++] = sq;
  }
  return i;
}

INLINE bb bb_get_pawns_attacks(int sq, int color) {
  assert(sq >= 0 && sq < SQUARE_NB);
  assert(color == WHITE || color == BLACK);
  return BB_PAWNS[color][sq];
}

INLINE bb bb_get_bishop_attacks(int sq, bb obs) {
  assert(sq >= 0 && sq < SQUARE_NB);
  return bb_bishop(sq, obs);
}

INLINE bb bb_get_knight_attacks(int sq) {
  assert(sq >= 0 && sq < SQUARE_NB);
  return BB_KNIGHT[sq];
}

INLINE bb bb_get_rook_attacks(int sq, bb obs) {
  assert(sq >= 0 && sq < SQUARE_NB);
  return bb_rook(sq, obs);
}

INLINE bb bb_get_queen_attacks(int sq, bb obs) {
  assert(sq >= 0 && sq < SQUARE_NB);
  return bb_queen(sq, obs);
}

INLINE bb bb_get_king_attacks(int sq) {
  assert(sq >= 0 && sq < SQUARE_NB);
  return BB_KING[sq];
}

int bb_attacks_to_king_square(ChessBoard *board, const bb b_king) {
  assert(b_king);
  return (bb_attacks_to_square(board, get_lsb(b_king), board->occ[BOTH]) &
          board->occ[board->color])
             ? 1
             : 0;
}

bool bb_is_check(ChessBoard *board) {
  bb them = board->occ[!board->color];
  int sq = get_lsb(board->bb_squares[board->color ? BLACK_KING : WHITE_KING]);

  return (bb_get_rook_attacks(sq, board->occ[BOTH]) &
          ((board->bb_squares[WHITE_ROOK] | board->bb_squares[BLACK_ROOK] |
            board->bb_squares[WHITE_QUEEN] | board->bb_squares[BLACK_QUEEN]) &
           them)) ||
         (bb_get_bishop_attacks(sq, board->occ[BOTH]) &
          ((board->bb_squares[WHITE_BISHOP] | board->bb_squares[BLACK_BISHOP] |
            board->bb_squares[WHITE_QUEEN] | board->bb_squares[BLACK_QUEEN]) &
           them)) ||
         (bb_get_knight_attacks(sq) &
          ((board->bb_squares[WHITE_KNIGHT] | board->bb_squares[BLACK_KNIGHT]) &
           them)) ||
         (bb_pawns_attacks(sq, board->color) &
          ((board->bb_squares[WHITE_PAWN] | board->bb_squares[BLACK_PAWN]) &
           them));
}

bb bb_attacks_to_square(ChessBoard *board, int sq, bb occ) {
  bb queens = board->bb_squares[WHITE_QUEEN] | board->bb_squares[BLACK_QUEEN];
  bb bishops =
      board->bb_squares[WHITE_BISHOP] | board->bb_squares[BLACK_BISHOP];
  bb rooks = board->bb_squares[WHITE_ROOK] | board->bb_squares[BLACK_ROOK];
  bb knights =
      board->bb_squares[WHITE_KNIGHT] | board->bb_squares[BLACK_KNIGHT];
  bb kings = board->bb_squares[WHITE_KING] | board->bb_squares[BLACK_KING];

  return (bb_get_pawns_attacks(sq, WHITE) & board->bb_squares[BLACK_PAWN]) |
         (bb_get_pawns_attacks(sq, BLACK) & board->bb_squares[WHITE_PAWN]) |
         (bb_get_knight_attacks(sq) & knights) |
         (bb_get_bishop_attacks(sq, occ) & bishops) |
         (bb_get_rook_attacks(sq, occ) & rooks) |
         (bb_get_queen_attacks(sq, occ) & queens) |
         (bb_get_king_attacks(sq) & kings);
}

bb bb_pawns_attacks(int sq, int color) {
  assert(sq >= 0 && sq < SQUARE_NB);
  const bb board = BIT(sq);
  return color ? ((board & ~FILE_H) >> 7) | ((board & ~FILE_A) >> 9)
               : ((board & ~FILE_A) << 7) | ((board & ~FILE_H) << 9);
}

bb bb_slide(int sq, int truncate, bb obs, int directions[4][2]) {
  bb value = 0;
  int rank = rank_of(sq);
  int file = file_of(sq);
  int i, n;
  for (i = 0; i < 4; i++) {
    bb prev = 0;
    for (n = 1; n < 9; n++) {
      int r = rank + directions[i][0] * n;
      int f = file + directions[i][1] * n;
      if (r < 0 || f < 0 || r > 7 || f > 7) {
        if (truncate)
          value &= ~prev;
        break;
      }
      bb bit = BIT(square(r, f));
      value |= bit;
      if (bit & obs)
        break;
      prev = bit;
    }
  }

  return value;
}

bb bb_slide_bishop(int sq, int truncate, bb obs) {
  int directions[4][2] = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};

  return bb_slide(sq, truncate, obs, directions);
}

bb bb_slide_rook(int sq, int truncate, bb obs) {
  int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
  return bb_slide(sq, truncate, obs, directions);
}

void bb_init() {
  // bb_pawns
  for (int sq = 0; sq < 64; sq++) {
    BB_PAWNS[WHITE][sq] = bb_pawns_attacks(sq, WHITE);
    BB_PAWNS[BLACK][sq] = bb_pawns_attacks(sq, BLACK);
  }

  // bb_knight
  const int knight_offsets[8][2] = {
      {-2, -1}, {-2, 1}, {2, -1}, {2, 1}, {-1, -2}, {-1, 2}, {1, -2}, {1, 2},
  };

  for (int rank = 0; rank < RANK_NB; rank++) {
    for (int file = 0; file < FILE_NB; file++) {
      bb value = 0;
      for (int i = 0; i < 8; i++) {
        int r = rank + knight_offsets[i][0];
        int f = file + knight_offsets[i][1];
        if (r >= 0 && f >= 0 && r < 8 && f < 8) {
          value |= BIT(square(r, f));
        }
      }
      BB_KNIGHT[square(rank, file)] = value;
    }
  }

  // bb_bishop
  for (int sq = 0; sq < SQUARE_NB; sq++) {
    BB_BISHOP[sq] = bb_slide_bishop(sq, 1, 0L);
    BB_ROOK[sq] = bb_slide_rook(sq, 1, 0L);
  }

  // attack_bishop
  int offset = 0;
  int squares[SQUARE_NB];
  for (int sq = 0; sq < SQUARE_NB; sq++) {
    int count = bb_squares(BB_BISHOP[sq], squares);
    int n = 1 << count;
    for (int i = 0; i < n; i++) {
      bb obs = 0;
      for (int j = 0; j < count; j++) {
        if (i & (1 << j)) {
          obs |= BIT(squares[j]);
        }
      }
      bb value = bb_slide_bishop(sq, 0, obs);
      int index = (obs * MAGIC_BISHOP[sq]) >> SHIFT_BISHOP[sq];
      bb prev = ATTACK_BISHOP[offset + index];
      if (prev && (prev != value)) {
        err("ERROR: invalid ATTACK_BISHOP table");
      }

      ATTACK_BISHOP[offset + index] = value;
    }

    OFFSET_BISHOP[sq] = offset;
    offset += 1 << (64 - SHIFT_BISHOP[sq]);
  }

  // attack_rook
  offset = 0;
  for (int sq = 0; sq < SQUARE_NB; sq++) {
    int count = bb_squares(BB_ROOK[sq], squares);
    int n = 1 << count;
    for (int i = 0; i < n; i++) {
      bb obstacles = 0;
      for (int j = 0; j < count; j++) {
        if (i & (1 << j)) {
          obstacles |= BIT(squares[j]);
        }
      }
      bb value = bb_slide_rook(sq, 0, obstacles);
      int index = (obstacles * MAGIC_ROOK[sq]) >> SHIFT_ROOK[sq];
      bb previous = ATTACK_ROOK[offset + index];
      if (previous && previous != value) {
        err("ERROR: invalid ATTACK_ROOK table");
      }
      ATTACK_ROOK[offset + index] = value;
    }
    OFFSET_ROOK[sq] = offset;
    offset += 1 << (64 - SHIFT_ROOK[sq]);
  }

  // BB_KING
  const int king_offsets[8][2] = {
      {-1, -1}, {0, -1}, {1, -1}, {-1, 0}, {1, 0}, {-1, 1}, {0, 1}, {1, 1},
  };
  for (int rank = 0; rank < RANK_NB; rank++) {
    for (int file = 0; file < FILE_NB; file++) {
      bb value = 0;
      for (int i = 0; i < 8; i++) {
        int r = rank + king_offsets[i][0];
        int f = file + king_offsets[i][1];
        if (r >= 0 && f >= 0 && r < 8 && f < 8) {
          value |= BIT(square(r, f));
        }
      }
      BB_KING[square(rank, file)] = value;
    }
  }
}

bb bb_bishop(int sq, bb obs) {
  bb value = obs & BB_BISHOP[sq];
  int index = (value * MAGIC_BISHOP[sq]) >> SHIFT_BISHOP[sq];
  return ATTACK_BISHOP[index + OFFSET_BISHOP[sq]];
}

bb bb_rook(int sq, bb obs) {
  bb value = obs & BB_ROOK[sq];
  int index = (value * MAGIC_ROOK[sq]) >> SHIFT_ROOK[sq];
  return ATTACK_ROOK[index + OFFSET_ROOK[sq]];
}

bb bb_queen(int sq, bb obs) { return bb_bishop(sq, obs) | bb_rook(sq, obs); }

void bb_print(bb bbit) {
  for (int r = RANK_NB - 1; r >= 0; r--) {
    for (int f = 0; f < FILE_NB; f++) {
      printf(" %c", (test_bit(bbit, square(r, f))) ? '1' : '.');
    }
    printf("\n");
  }
  printf("\n");
}