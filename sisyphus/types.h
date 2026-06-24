#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>
#include <stdint.h>

#define INLINE inline __attribute__((always_inline))

// Debug print macro
#if !defined(DEBUG_DISABLE_PRINT) || defined(DEBUG)
#define err(str)                                                               \
  fprintf(stderr, "%s, at %s, line %d\n", str, __FILE__, __LINE__);            \
  fflush(stderr)
#else
#define err(str)
#endif

// Assertion macro
#if !defined(DISABLE_ASSERT)
#define ASSERT(expr) assert(expr)
#else
#define ASSERT(expr) ((void)0)
#endif

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(*(arr)))

typedef unsigned long long
    bb; // Bitboard type definition (64-bit unsigned integer)

#define U64(u) u##ULL
#define U32(u) u##U

typedef struct {
  int squares[64]; // Piece placement array
  int numMoves;    // Number of moves played
  int color;       // Current side to move
  int castle;      // Castling rights

  int mg[2];     // Middlegame evaluation score for both sides
  int eg[2];     // Endgame evaluation score for both sides
  int gamePhase; // Current game phase (opening/middlegame/endgame)

  bb bb_squares[12];  // Bitboards for each piece type and color
  bb m_history[8192]; // Move history for repetition detection
  bb occ[3];          // Occupancy bitboards (white/black/both)

  bb ep;        // En passant square bitboard
  bb hash;      // Position hash
  bb pawn_hash; // Pawn structure hash (not used)
} ChessBoard;

typedef uint32_t Move; // Move type (32-bit unsigned integer)

typedef int (*MoveGen)(ChessBoard *, Move *);
typedef int (*AttacksGen)(ChessBoard *, Move *, bb);

typedef struct {
  int capture; // Captured piece
  int castle;  // Previous castling rights
  bb ep;       // Previous en passant square
} Undo;

typedef struct {
  bb key;    // Position hash
  int score; // Evaluation score
  int depth; // Search depth
  int flag;  // Entry type flag
  Move move; // Best move
} Entry;

typedef struct {
  int size;     // Table size
  int mask;     // Size mask for indexing
  Entry *entry; // Array of entries
} Table;

typedef struct {
  bb key;    // Pawn structure hash
  int value; // Evaluation score
} PawnEntry;

typedef struct {
  int size;         // Table size
  int mask;         // Size mask for indexing
  PawnEntry *entry; // Array of entries
} PawnTable;

typedef struct {
  int nodes;   // Nodes searched
  bool stop;   // Search stop flag
  Move move;   // Best move found
  Table table; // Transposition table
} Search;

typedef struct {
  int score;         // Evaluation score
  bool debug;        // Debug flag
  Search *search;    // Search information
  ChessBoard *board; // Board state
  Move *move;        // Move to be played
} Thread_d;

typedef struct {
  bb key;    // Position hash
  bb value;  // Stored value
  int depth; // Search depth
} Entry_t;

enum Square {
  A1 = 1,
  B1,
  C1,
  D1,
  E1,
  F1,
  G1,
  H1,
  A2,
  B2,
  C2,
  D2,
  E2,
  F2,
  G2,
  H2,
  A3,
  B3,
  C3,
  D3,
  E3,
  F3,
  G3,
  H3,
  A4,
  B4,
  C4,
  D4,
  E4,
  F4,
  G4,
  H4,
  A5,
  B5,
  C5,
  D5,
  E5,
  F5,
  G5,
  H5,
  A6,
  B6,
  C6,
  D6,
  E6,
  F6,
  G6,
  H6,
  A7,
  B7,
  C7,
  D7,
  E7,
  F7,
  G7,
  H7,
  A8,
  B8,
  C8,
  D8,
  E8,
  F8,
  G8,
  H8,
};

enum BoardDimensions { SQUARE_NB = 64, COLOR_NB = 2, FILE_NB = 8, RANK_NB = 8 };

enum PieceValue {
  PAWN_MATERIAL = 100,
  KNIGHT_MATERIAL = 320,
  BISHOP_MATERIAL = 330,
  ROOK_MATERIAL = 500,
  QUEEN_MATERIAL = 900,
  KING_MATERIAL = 20000
};

enum Color { WHITE = 0, BLACK = 1, BOTH = 2 };

enum PieceType {
  PAWN = 0,
  KNIGHT = 1,
  BISHOP = 2,
  ROOK = 3,
  QUEEN = 4,
  KING = 5
};

enum ColoredPiece {
  WHITE_PAWN = 0,
  BLACK_PAWN = 1,
  WHITE_KNIGHT = 2,
  BLACK_KNIGHT = 3,
  WHITE_BISHOP = 4,
  BLACK_BISHOP = 5,
  WHITE_ROOK = 6,
  BLACK_ROOK = 7,
  WHITE_QUEEN = 8,
  BLACK_QUEEN = 9,
  WHITE_KING = 10,
  BLACK_KING = 11,
  NONE = 12
};

enum CastleRights {
  CASTLE_WHITE_KING_SIDE = 1 << 0,
  CASTLE_WHITE_QUEEN_SIDE = 1 << 1,
  CASTLE_BLACK_KING_SIDE = 1 << 2,
  CASTLE_BLACK_QUEEN_SIDE = 1 << 3,

  CASTLE_WHITE = CASTLE_WHITE_KING_SIDE | CASTLE_WHITE_QUEEN_SIDE,
  CASTLE_BLACK = CASTLE_BLACK_KING_SIDE | CASTLE_BLACK_QUEEN_SIDE,
  CASTLE_ALL = CASTLE_WHITE | CASTLE_BLACK
};

#define RANK_1 0x00000000000000ffULL
#define RANK_2 0x000000000000ff00ULL
#define RANK_3 0x0000000000ff0000ULL
#define RANK_4 0x00000000ff000000ULL
#define RANK_5 0x000000ff00000000ULL
#define RANK_6 0x0000ff0000000000ULL
#define RANK_7 0x00ff000000000000ULL
#define RANK_8 0xff00000000000000ULL

#define FILE_A 0x0101010101010101ULL
#define FILE_B 0x0202020202020202ULL
#define FILE_C 0x0404040404040404ULL
#define FILE_D 0x0808080808080808ULL
#define FILE_E 0x1010101010101010ULL
#define FILE_F 0x2020202020202020ULL
#define FILE_G 0x4040404040404040ULL
#define FILE_H 0x8080808080808080ULL

#endif // TYPES_H