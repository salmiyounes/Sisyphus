"""
Python Chess Engine Interface

This module connects Python to a chess engine written in C using ctypes.
It includes tools for working with chess boards, pieces, and moves.

Main features:
- Create and manage chess games
- Generate legal and pseudo-legal moves
- Handle special rules like castling and en passant
- Convert positions to and from FEN
- Search for the best move using the engine

"""

from __future__ import annotations

__author__ = "Salmi Younes"

__version__ = "0.1.0"

__license__ = "MIT"

import os
import dataclasses
from typing import (
    Optional,
    Callable,
    Union,
    List,
    Tuple,
    TypeAlias,
    Iterator,
    Self,
    Any,
)
from ctypes import (
    CDLL,
    Structure,
    POINTER,
    pointer,
    byref,
    Array,
    c_uint64,
    c_uint32,
    c_char,
    c_char_p,
    c_int,
    c_bool,
    c_void_p,
    c_float,
    create_string_buffer,
)

# Starting possition fen
STARTING_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -"

_bb_initialized = False

BitBoard: TypeAlias = int

Square: TypeAlias = int
SQUARE_NB: Square = 64
A1: Square = 0
B1: Square = 1
C1: Square = 2
D1: Square = 3
E1: Square = 4
F1: Square = 5
G1: Square = 6
H1: Square = 7
A2: Square = 8
B2: Square = 9
C2: Square = 10
D2: Square = 11
E2: Square = 12
F2: Square = 13
G2: Square = 14
H2: Square = 15
A3: Square = 16
B3: Square = 17
C3: Square = 18
D3: Square = 19
E3: Square = 20
F3: Square = 21
G3: Square = 22
H3: Square = 23
A4: Square = 24
B4: Square = 25
C4: Square = 26
D4: Square = 27
E4: Square = 28
F4: Square = 29
G4: Square = 30
H4: Square = 31
A5: Square = 32
B5: Square = 33
C5: Square = 34
D5: Square = 35
E5: Square = 36
F5: Square = 37
G5: Square = 38
H5: Square = 39
A6: Square = 40
B6: Square = 41
C6: Square = 42
D6: Square = 43
E6: Square = 44
F6: Square = 45
G6: Square = 46
H6: Square = 47
A7: Square = 48
B7: Square = 49
C7: Square = 50
D7: Square = 51
E7: Square = 52
F7: Square = 53
G7: Square = 54
H7: Square = 55
A8: Square = 56
B8: Square = 57
C8: Square = 58
D8: Square = 59
E8: Square = 60
F8: Square = 61
G8: Square = 62
H8: Square = 63

# Max moves
MAX_MOVES: int = 218

# Color number
COLOR_NB: int = 2

# Colors
Color: TypeAlias = int
WHITE: Color = 0
BLACK: Color = 1
BOTH: Color = 2

# Flags
Flags: TypeAlias = int
EMPTY_FLAG: Flags = 0
ENP_FLAG: Flags = 6
CASTLE_FLAG: Flags = 1
PROMO_FLAG: Flags = 8
KNIGHT_PROMO_FLAG: Flags = 8
ROOK_PROMO_FLAG: Flags = 9
BISHOP_PROMO_FLAG: Flags = 10
QUEEN_PROMO_FLAG: Flags = 11

# Castling rights
CASTLE_ALL: Flags = 15
CASTLE_WHITE: Flags = 3
CASTLE_BLACK: Flags = 12
CASTLE_WHITE_KING_SIDE: Flags = 1
CASTLE_WHITE_QUEEN_SIDE: Flags = 2
CASTLE_BLACK_KING_SIDE: Flags = 4
CASTLE_BLACK_QUEEN_SIDE: Flags = 8

# piece
Piece: TypeAlias = int
PAWN: Piece = 0
KNIGHT: Piece = 1
BISHOP: Piece = 2
ROOK: Piece = 3
QUEEN: Piece = 4
KING: Piece = 5
NONE: Piece = 12

# Piece symbols list
PIECE_SYMBOLS: List[str] = ["p", "n", "b", "r", "q", "k"]


class ChessBoard(Structure):
    """A C structure that represents a chess position using bitboards for piece placement, game state, and move history."""

    _fields_ = [
        ("squares", c_int * SQUARE_NB),
        ("numMoves", c_int),
        ("color", c_int),
        ("castle", c_int),
        ("mg", c_int * 2),
        ("eg", c_int * 2),
        ("gamePhase", c_int),
        ("bb_squares", c_uint64 * 12),
        ("m_history", c_uint64 * 8192),
        ("occ", c_uint64 * 3),
        ("ep", c_uint64),
        ("hash", c_uint64),
        ("pawn_hash", c_uint64),
    ]


class Entry(Structure):
    """A C structure that represents a transposition table entry storing position evaluations and best moves."""

    _fields_ = [
        ("key", c_uint64),
        ("score", c_int),
        ("depth", c_int),
        ("flag", c_int),
    ]


class Table(Structure):
    """A C structure representing a transposition table with size, mask and array of position entries."""

    _fields_ = [("size", c_int), ("mask", c_int), ("entry", POINTER(Entry))]


class Search(Structure):
    """A C structure that manages chess engine search state including node count, stop flag, and transposition table."""

    _fields_ = [("nodes", c_int), ("stop", c_bool), ("num", c_int), ("table", Table)]


class Undo(Structure):
    """A C structure that stores information needed to undo a chess move, including captured pieces, castling rights, and en passant state."""

    _fields_ = [("capture", c_int), ("castle", c_int), ("ep", c_uint64)]


try:
    chess_lib = CDLL(os.path.join(os.path.dirname(__file__), "libchess.so"))
except OSError as e:
    raise RuntimeError(f"Could not load chess engine library: {e}")


# Bitboard manipulation functions
chess_lib.get_lsb.argtypes = [c_uint64]
chess_lib.get_lsb.restype = c_int
chess_lib.get_msb.argtypes = [c_uint64]
chess_lib.get_msb.restype = c_int
chess_lib.popcount.argtypes = [c_uint64]
chess_lib.popcount.restype = c_int
chess_lib.several.argtypes = [c_uint64]
chess_lib.several.restype = c_int
chess_lib.test_bit.argtypes = [c_uint64, c_int]
chess_lib.test_bit.restype = c_bool
chess_lib.square.argtypes = [c_int, c_int]
chess_lib.square.restype = c_int
chess_lib.file_of.argtypes = [c_int]
chess_lib.file_of.restype = c_int
chess_lib.rank_of.argtypes = [c_int]
chess_lib.rank_of.restype = c_int
chess_lib.bb_init.argtypes = []
chess_lib.bb_init.restype = c_void_p
chess_lib.bb_print.argtypes = [c_uint64]
chess_lib.bb_print.restype = c_void_p
chess_lib.bb_attacks_to_king_square.argtypes = [POINTER(ChessBoard), c_uint64]
chess_lib.bb_attacks_to_king_square.restype = c_int
chess_lib.bb_attacks_to_square.argtypes = [POINTER(ChessBoard), c_int, c_uint64]
chess_lib.bb_attacks_to_square.restype = c_uint64

# Main board functions
chess_lib.board_init.argtypes = [POINTER(ChessBoard)]
chess_lib.board_init.restype = c_void_p
chess_lib.board_load_fen.argtypes = [POINTER(ChessBoard), POINTER(c_char)]
chess_lib.board_load_fen.restype = c_void_p
chess_lib.print_board.argtypes = [POINTER(ChessBoard)]
chess_lib.print_board.restype = c_void_p
chess_lib.board_to_fen.argtypes = [POINTER(ChessBoard), POINTER(c_char)]
chess_lib.board_to_fen.restype = c_void_p
chess_lib.perft_test.argtypes = [POINTER(ChessBoard), c_int]
chess_lib.perft_test.restype = c_uint64
chess_lib.board_clear.argtypes = [POINTER(ChessBoard)]
chess_lib.board_clear.restype = c_void_p
chess_lib.board_drawn_by_insufficient_material.argtypes = [POINTER(ChessBoard)]
chess_lib.board_drawn_by_insufficient_material.restype = c_int

# Move Generation functions
chess_lib.gen_black_attacks_against.argtypes = [
    POINTER(ChessBoard),
    POINTER(c_uint32),
    c_uint64,
]
chess_lib.gen_black_attacks_against.restype = c_int
chess_lib.gen_white_attacks_against.argtypes = [
    POINTER(ChessBoard),
    POINTER(c_uint32),
    c_uint64,
]
chess_lib.gen_white_attacks_against.restype = c_int
chess_lib.gen_attacks.argtypes = [POINTER(ChessBoard), POINTER(c_uint32)]
chess_lib.gen_attacks.restype = c_int
chess_lib.gen_moves.argtypes = [POINTER(ChessBoard), POINTER(c_uint32)]
chess_lib.gen_moves.restype = c_int
chess_lib.gen_legal_moves.argtypes = [POINTER(ChessBoard), POINTER(c_uint32)]
chess_lib.gen_legal_moves.restype = c_int
chess_lib.illegal_to_move.argtypes = [POINTER(ChessBoard)]
chess_lib.illegal_to_move.restype = c_int
chess_lib.is_check.argtypes = [POINTER(ChessBoard)]
chess_lib.is_check.restype = c_int

# Move Handling functions
chess_lib.do_move.argtypes = [POINTER(ChessBoard), c_uint32, POINTER(Undo)]
chess_lib.do_move.restype = c_void_p
chess_lib.undo_move.argtypes = [POINTER(ChessBoard), c_uint32, POINTER(Undo)]
chess_lib.undo_move.restype = c_void_p
chess_lib.move_to_str.argtypes = [c_uint32]
chess_lib.move_to_str.restype = c_char_p
chess_lib.make_move.argtypes = [POINTER(ChessBoard), c_uint32]
chess_lib.make_move.restype = c_void_p

# Search and functions
chess_lib.thread_init.argtypes = [
    POINTER(Search),
    POINTER(ChessBoard),
    POINTER(c_uint32),
    c_float,
    c_bool,
]
chess_lib.thread_init.restype = c_void_p
chess_lib.thread_stop.argtypes = [POINTER(Search)]
chess_lib.thread_stop.restype = c_void_p
chess_lib.best_move.argtypes = [POINTER(Search), POINTER(ChessBoard), POINTER(c_uint32)]
chess_lib.best_move.restype = c_int


class IllegalMoveError(ValueError):
    """Exception raised when attempting to make an illegal chess move."""

    pass


class utils:
    """A collection of static utility methods for chess operations.

    This class provides helper methods for bit manipulation, square coordinates,
    move encoding and other chess-related calculations.
    """

    @staticmethod
    def create_string_buffer(size: int) -> Array[Any]:
        """Create an array of unsigned 32-bit integers.

        Args:
            size: The size of the array to create

        Returns:
            A new uint32 array
        """
        return create_string_buffer(size)

    @staticmethod
    def create_uint32_array(size: int) -> Array[Any]:
        """Create an array of unsigned 32-bit integers.

        Args:
            size: The size of the array to create

        Returns:
            A new uint32 array
        """
        return (c_uint32 * size)()

    @staticmethod
    def get_lsb(bbit: BitBoard) -> int:
        """Get the least significant set bit in a bitboard.

        Args:
            bbit: The bitboard to examine

        Returns:
            The index of the least significant set bit
        """
        lsb = chess_lib.get_lsb(bbit)
        return int(lsb)

    @staticmethod
    def get_msb(bbit: BitBoard) -> int:
        """Get the most significant set bit in a bitboard.

        Args:
            bbit: The bitboard to examine

        Returns:
            The index of the most significant set bit
        """
        msb = chess_lib.get_msb(bbit)
        return int(msb)

    @staticmethod
    def popcount(bbit: BitBoard) -> int:
        """Count the number of set bits in a bitboard.

        Args:
            bbit: The bitboard to count bits in

        Returns:
            The number of set bits
        """
        count = chess_lib.popcount(bbit)
        return int(count)

    @staticmethod
    def test_bit(bbit: BitBoard, sq: Square) -> bool:
        """Test if a specific bit is set in a bitboard.

        Args:
            bbit: The bitboard to test
            sq: The square index to check

        Returns:
            True if the bit is set, False otherwise
        """
        bit = chess_lib.test_bit(bbit, sq)
        return bool(bit)

    @staticmethod
    def square(rank: Square, file: Square) -> Square:
        """Convert rank and file coordinates to a square index.

        Args:
            rank: The rank coordinate (0-7)
            file: The file coordinate (0-7)

        Returns:
            The square index (0-63)
        """
        sq = chess_lib.square(rank, file)
        return Square(sq)

    @staticmethod
    def file_of(sq: Square) -> int:
        """Get the file of a square.

        Args:
            sq: The square index

        Returns:
            The file number (0-7)
        """
        file = chess_lib.file_of(sq)
        return int(file)

    @staticmethod
    def rank_of(sq: Square) -> int:
        """Get the rank of a square.

        Args:
            sq: The square index

        Returns:
            The rank number (0-7)
        """
        rank = chess_lib.rank_of(sq)
        return int(rank)

    @staticmethod
    def bit(sq: Square) -> int:
        """Convert a square index to a bitboard with only that bit set.

        Args:
            sq: The square index

        Returns:
            A bitboard with only the specified bit set
        """
        return 1 << sq

    @staticmethod
    def scan_forward(bb: BitBoard) -> Iterator[Square]:
        """Iterate through set bits in a bitboard from LSB to MSB.

        Args:
            bb: The bitboard to scan

        Yields:
            Square indices of set bits
        """
        while bb:
            r = bb & -bb
            yield r.bit_length() - 1
            bb ^= r

    @staticmethod
    def square_distance(a: Square, b: Square) -> int:
        """Calculate the Chebyshev distance between two squares.

        Args:
            a: First square index
            b: Second square index

        Returns:
            The number of king moves needed to go from a to b
        """
        return max(
            abs(utils.file_of(a) - utils.file_of(b)),
            abs(utils.rank_of(a) - utils.rank_of(b)),
        )

    @staticmethod
    def square_manhattan_distance(a: Square, b: Square) -> int:
        """Calculate the Manhattan distance between two squares.

        Args:
            a: First square index
            b: Second square index

        Returns:
            The sum of horizontal and vertical distances
        """
        return abs(utils.file_of(a) - utils.file_of(b)) + abs(
            utils.rank_of(a) - utils.rank_of(b)
        )

    @staticmethod
    def square_mirror(square: Square) -> Square:
        """Mirror a square vertically on the board.

        Args:
            square: The square index to mirror

        Returns:
            The mirrored square index
        """
        return square ^ 56

    @staticmethod
    def encode_move(from_sq: int, to_sq: int, piece: int, flag: int) -> int:
        """Encode move information into a single integer.

        Args:
            from_sq: Source square (0-63)
            to_sq: Target square (0-63)
            piece: Piece type and color
            flag: Move flags (castling, en passant, promotion)

        Returns:
            Encoded move as a 32-bit integer
        """
        return (
            (from_sq & 0x3F)
            | ((to_sq & 0x3F) << 6)
            | ((piece & 0xF) << 12)
            | ((flag & 0xF) << 16)
        )

    @staticmethod
    def scan_move_list(arr: List[int]) -> Iterator[List[Any]]:
        """Convert a list of encoded moves into move components.

        Args:
            arr: List of encoded move integers

        Yields:
            Lists containing [source square, target square, piece type, flags]
        """

        # Helper Functions to extract specific fields
        def extract_source_sq(x: int) -> Square:
            return (x >> 0) & 0x3F

        def extract_target_sq(x: int) -> Square:
            return (x >> 6) & 0x3F

        def extract_piece_type(x: int) -> Optional[PieceType]:
            index: int = (x >> 12) & 0xF
            return PieceType.from_index(index)

        def extract_flag_type(x: int) -> Flags:
            return (x >> 16) & 0xF

        extractors: List[Callable[[int], Union[Square, Optional[PieceType], Flags]]] = [
            extract_source_sq,
            extract_target_sq,
            extract_piece_type,
            extract_flag_type,
        ]

        for m in arr:
            yield [extr(m) for extr in extractors]


@dataclasses.dataclass
class PieceType:
    """Represents a chess piece with its type and color.

    This class encapsulates both the piece type (pawn, knight, bishop, etc.)
    and its color (white or black). It provides methods for converting between
    different piece representations and generating piece symbols.

    Attributes:
        piece (Piece): The type of piece (PAWN, KNIGHT, BISHOP, ROOK, QUEEN, or KING)
        color (Color): The piece color (WHITE or BLACK)

    Example:
        >>> piece = PieceType(KNIGHT, WHITE)
        PieceType'n'
        >>> print(piece.symbol())
        'n'
        >>> print(PieceType.from_symbol('p'))
        PieceType'p'
        >>>
    """

    piece: Piece
    """The type of piece (0=PAWN through 5=KING)"""

    color: Color
    """The piece color (0=WHITE, 1=BLACK)"""

    def symbol(self) -> str:
        """Get the piece's symbol in algebraic notation.

        Returns:
            str: Uppercase letter for white pieces, lowercase for black pieces.
                 P=pawn, N=knight, B=bishop, R=rook, Q=queen, K=king
        """
        return (
            PIECE_SYMBOLS[self.piece].upper()
            if not self.color
            else PIECE_SYMBOLS[self.piece]
        )

    def __repr__(self) -> str:
        return f"{type(self).__name__}{self.symbol()!r}"

    def __hash__(self) -> int:
        """Generate a unique hash value for the piece.

        Combines piece type and color into a single integer for use as dictionary key.

        Returns:
            int: Unique hash value for this piece type and color combination
        """
        return (self.piece * BOTH) + self.color

    @classmethod
    def from_index(cls, index: int) -> Optional[PieceType]:
        """Create a PieceType from an integer index representation.

        Args:
            index (int): Integer encoding both piece type and color

        Returns:
            Optional[PieceType]: New PieceType instance, or None if index represents NONE
        """
        if index != NONE:  # Check if 'index' is not a NONE piece type
            return cls(
                (index & ~BLACK) >> BLACK,  # Get piece type
                index & BLACK,  # Get color type
            )
        else:
            return None

    @classmethod
    def from_symbol(cls, symbol: str) -> PieceType:
        """Create a PieceType from a piece symbol.

        Args:
            symbol (str): Single character piece symbol (e.g. 'P', 'n', 'K')
                         Uppercase for white pieces, lowercase for black

        Returns:
            PieceType: New PieceType instance representing the piece

        Example:
            >>> PieceType.from_symbol('Q')  # Black queen
            PieceType(piece=4, color=1)
            >>> PieceType.from_symbol('p')  # White pawn
            PieceType(piece=0, color=0)
        """
        return cls(PIECE_SYMBOLS.index(symbol.lower()), not symbol.islower())


@dataclasses.dataclass(slots=True)
class Move:
    """Represents a chess move from one square to another.

    This class encapsulates all information about a chess move including the source and
    destination squares, the piece being moved, and special move flags for castling,
    en passant, and promotions.

    Attributes:
        from_sq (Square): The source square index (0-63)
        dst_sq (Square): The destination square index (0-63)
        piece (Optional[PieceType]): The piece being moved, or None
        flag (Flags): Special move flags:
            - EMPTY_FLAG (0): Normal move
            - CASTLE_FLAG (1): Castling move
            - ENP_FLAG (6): En passant capture
            - PROMO_FLAG (8): Pawn promotion base flag
            - KNIGHT_PROMO_FLAG (8): Promote to knight
            - BISHOP_PROMO_FLAG (10): Promote to bishop
            - ROOK_PROMO_FLAG (9): Promote to rook
            - QUEEN_PROMO_FLAG (11): Promote to queen

    Examples:
        >>> # Create a normal pawn move
        >>> move = Move(E2, E4, PieceType(PAWN, WHITE))
        Move(san=e2e4, from=12, to=28, piece=PieceType'p', flag=0)
        >>> print(move.san)
        'e2e4'

        >>> # Create a promotion move
        >>> move = Move(H7, H8, PieceType(PAWN, WHITE), PROMO_FLAG | QUEEN_PROMO_FLAG)
        >>> print(move.san)
        'h7h8q'
    """

    from_sq: Square
    """Source square"""

    dst_sq: Square
    """Destenation square"""

    piece: Optional[PieceType] = None
    """Piece type"""

    flag: Flags = EMPTY_FLAG
    """Flag type"""

    def __repr__(self) -> str:
        return f"{type(self).__name__}(san={self.san}, from={self.from_sq}, to={self.dst_sq}, piece={self.piece}, flag={self.flag})"

    def __str__(self) -> str:
        return self.san

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, Move):
            return NotImplemented
        return (
            self.dst_sq == other.dst_sq
            and self.from_sq == other.from_sq
            and self.piece == other.piece
            and self.flag == other.flag
        )

    def __hash__(self) -> int:
        """Generate a unique hash value for the move.

        Encodes the move information into a 32-bit integer using the format:
        - bits 0-5: source square (0-63)
        - bits 6-11: destination square (0-63)
        - bits 12-15: piece type and color
        - bits 16-19: move flags

        Returns:
            int: Unique hash value for this move
        """
        return utils.encode_move(
            self.from_sq, self.dst_sq, hash(self.piece) if self.piece else 0, self.flag
        )

    def move_str(self) -> str:
        """Get the move in standard algebraic notation (SAN).

        Returns:
            str: The move in SAN format
        """
        try:
            result = chess_lib.move_to_str(hash(self))
            if not result:
                return ""
            return str(result.decode("utf-8"))
        except Exception as e:
            print(f"Error converting move to string: {e}")
            return ""

    @classmethod
    def parse_uci(cls, board: Board, uci: str) -> Move:
        if not isinstance(uci, str):
            raise ValueError("UCI must be a string")
        if len(uci) not in (4, 5):
            raise ValueError(f"Expected UCI string of length 4 or 5: {uci!r}")

        # file+rank -> 0..63
        def square(s: str) -> Square:
            f = ord(s[0]) - ord("a")
            r = int(s[1]) - 1
            if not (0 <= f <= 7 and 0 <= r <= 7):
                raise ValueError(f"Invalid square in UCI: {s!r}")
            return r * 8 + f

        from_sq = square(uci[0:2])
        to_sq = square(uci[2:4])

        # make sure there's something to move
        piece = board.board.piece_at(from_sq)
        if piece is None:
            raise ValueError(f"No piece on source square {uci[0:2]!r}")

        flag = EMPTY_FLAG

        # promotion?
        if len(uci) == 5:
            promo = uci[4].lower()
            if promo == "n":
                flag = KNIGHT_PROMO_FLAG
            elif promo == "b":
                flag = BISHOP_PROMO_FLAG
            elif promo == "r":
                flag = ROOK_PROMO_FLAG
            elif promo == "q":
                flag = QUEEN_PROMO_FLAG
            else:
                raise ValueError(f"Invalid promotion piece: {promo!r}")

        # castling? (king moves two files)
        if piece.piece == KING and abs((to_sq % 8) - (from_sq % 8)) == 2:
            flag = CASTLE_FLAG

        # en-passant? (pawn captures diagonally onto empty square)
        file_diff = abs((to_sq % 8) - (from_sq % 8))
        if (
            piece.piece == PAWN
            and file_diff == 1
            and board.board.piece_at(to_sq) is None
        ):
            flag = ENP_FLAG

        return cls(from_sq, to_sq, piece, flag)

    def promo_piece_type(self) -> Optional[Piece]:
        """Get the piece type for a promotion move.

        Returns:
            Optional[Piece]: The promotion piece type (KNIGHT, BISHOP, ROOK, QUEEN),
                           or None if this is not a promotion move
        """
        if self.is_promotion():
            return (self.flag & 0x3) + KNIGHT
        else:
            return None

    def is_promotion(self) -> bool:
        """Check if this is a pawn promotion move.

        Returns:
            bool: True if this move is a pawn promotion
        """
        return bool(self.flag & PROMO_FLAG)

    def is_enp(self) -> bool:
        """Check if this is an en passant capture.

        Returns:
            bool: True if this move is an en passant capture
        """
        return bool(self.flag == ENP_FLAG)

    def is_castling(self) -> bool:
        """Check if this is a castling move.

        Returns:
            bool: True if this move is a castling move (kingside or queenside)
        """
        return bool(self.flag == CASTLE_FLAG)

    @property
    def san(self) -> str:
        """The move in standard algebraic notation.

        Returns:
            str: Move in SAN format (e.g. 'e4', 'Nf3')
        """
        return self.move_str()

    @classmethod
    def null(cls) -> Move:
        """Create a null move.

        Returns:
            Move: A move from square 0 to 0 with no piece or flags
        """
        return cls(0, 0)


class SquareSet:
    """Represents a set of squares on a chess board using a bitboard.

    A SquareSet uses a 64-bit integer (bitboard) where each bit represents a square
    on the chess board. This allows for efficient operations on sets of squares.

    The least significant bit represents square A1, and the most significant bit
    represents square H8:

        8 | 56 57 58 59 60 61 62 63
        7 | 48 49 50 51 52 53 54 55
        6 | 40 41 42 43 44 45 46 47
        5 | 32 33 34 35 36 37 38 39
        4 | 24 25 26 27 28 29 30 31
        3 | 16 17 18 19 20 21 22 23
        2 |  8  9 10 11 12 13 14 15
        1 |  0  1  2  3  4  5  6  7
          -------------------------
            a  b  c  d  e  f  g  h

    Attributes:
        mask (BitBoard): The underlying 64-bit integer representing the set of squares
    """

    def __init__(self, mask: Optional[BitBoard] = 0):
        self.mask = 0 if mask is None else mask

    def __len__(self) -> int:
        """Return the number of squares in the set.

        Returns:
            Number of bits set to 1 in the bitboard
        """
        return int(utils.popcount(self.mask))

    def __str__(self) -> str:
        """Print an ASCII representation of the bitboard.

        Returns:
            Empty string after printing the board
        """
        chess_lib.bb_print(self.mask)
        return ""

    def __repr__(self) -> str:
        return f"SquareSet({self.mask:#021_x})"

    def __add__(self, other: object) -> SquareSet:
        if not isinstance(other, SquareSet):
            return NotImplemented
        return SquareSet(self.mask | other.mask)

    def __and__(self, other: object) -> SquareSet:
        if not isinstance(other, SquareSet):
            return NotImplemented
        return SquareSet(other.mask & self.mask)

    def __xor__(self, other: object) -> SquareSet:
        if not isinstance(other, SquareSet):
            return NotImplemented
        return SquareSet(self.mask ^ other.mask)

    def __int__(self) -> int:
        return self.mask

    def __iter__(self) -> Iterator[Square]:
        return utils.scan_forward(self.mask)

    def __bool__(self) -> bool:
        return bool(self.mask)

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, SquareSet):
            return NotImplemented
        return self.mask == other.mask

    def __contains__(self, sq: Square) -> bool:
        return bool(utils.bit(sq) & int(self))

    def pop(self) -> Square:
        """Remove and return the least significant set square.

        Returns:
            Index of the least significant set bit

        Raises:
            KeyError: If the set is empty
        """
        if not self.mask:
            raise KeyError("pop from empty SquareSet")
        square: Square = utils.get_lsb(self.mask)
        self.mask &= self.mask - 1
        return square

    def clear(self) -> None:
        """Remove all squares from the set."""
        self.mask = BitBoard(0)

    def copy(self) -> SquareSet:
        """Create a copy of this SquareSet.

        Returns:
            New SquareSet with the same bits set
        """
        return SquareSet(self.mask)

    def tolist(self) -> List[bool]:
        """Convert the bitboard to a list of boolean values.

        Returns:
            List of 64 booleans indicating which squares are set
        """
        arr: List[bool] = [False] * SQUARE_NB
        for sq in self:
            arr[sq] = True
        return arr


class Searcher:
    """Chess engine searcher that manages search parameters and execution."""

    def __init__(self, board: Board, debug: bool = False):
        """Initialize the searcher with a board position.

        Args:
            board: The chess board to analyze
            debug: Enable debug output during search
        """
        self.board = board
        self.search = Search()
        self.move = c_uint32()
        self.debug = debug
        self._is_searching = False

    def start(self, depth: int = 6, time_s: float = 1.0) -> Move:
        """Start the search for the best move.

        Args:
            depth: Maximum search depth (default 6 ply)

        Returns:
            The best move found

        Raises:
            ValueError: If depth is not positive
        """
        if depth <= 0:
            raise ValueError("Search depth must be positive")

        if self._is_searching:
            raise RuntimeError("Search already in progress")

        self._is_searching = True
        try:
            # Initialize search thread
            chess_lib.thread_init(
                byref(self.search),
                self.board.board.ptr,
                byref(self.move),
                time_s,
                self.debug,
            )

            # Convert move value to Move object
            best_move = self._convert_move(self.move.value)

            return best_move

        finally:
            self._is_searching = False

    def stop(self) -> None:
        """Stop the current search."""
        if self._is_searching:
            # Stop the search
            chess_lib.thread_stop(byref(self.search))
            self._is_searching = False

    def clear(self) -> None:
        """Clear search state and hash tables."""
        self.search = Search()
        self.move = c_uint32()
        self._is_searching = False

    def _convert_move(self, move_val: int) -> Move:
        """Convert raw move value to Move object."""
        if not move_val:
            return Move.null()

        from_sq = (move_val >> 0) & 0x3F
        to_sq = (move_val >> 6) & 0x3F
        piece_val = (move_val >> 12) & 0xF
        flag = (move_val >> 16) & 0xF

        piece = PieceType.from_index(piece_val) if piece_val != NONE else None

        return Move(from_sq, to_sq, piece, flag)

    @property
    def nodes(self) -> int:
        """Number of nodes searched."""
        nodes = self.search.nodes
        return int(nodes)

    @property
    def is_searching(self) -> bool:
        """Whether a search is currently in progress."""
        return self._is_searching


class BaseBoard:
    """A low-level chess board representation using bitboards.

    The BaseBoard class provides core functionality for representing and manipulating
    a chess position using bitboards. Each piece type and color combination is
    represented by a 64-bit integer where each bit corresponds to a square.

    This class handles:
    - Piece placement and removal
    - Board initialization
    - Bitboard operations for pieces and squares
    - Attack generation
    - Position comparison and hashing

    The board is represented internally using the ChessBoard C structure through ctypes.

    Attributes:
        baseboard (ChessBoard): Internal C structure representing the board state
    """

    def __init__(self) -> None:
        self.baseboard: ChessBoard = ChessBoard()
        self.board_init()

    def __repr__(self) -> str:
        return f"{type(self).__name__}({self.occ(BOTH).mask:#021_x})"

    def __str__(self) -> str:
        mask: BitBoard = self.occ(BOTH).mask
        chess_lib.bb_print(mask)
        return ""

    def __iter__(self) -> Iterator[Square]:
        """Iterate through all occupied squares on the board.

        Yields:
            Square: Index of each occupied square (0-63)
        """
        mask: BitBoard = self.occ(BOTH).mask
        return utils.scan_forward(mask)

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, BaseBoard):
            return NotImplemented

        return (
            self.baseboard.castle == other.baseboard.castle
            and self.baseboard.ep == other.baseboard.ep
            and all(self.piece_at(sq) == other.piece_at(sq) for sq in range(SQUARE_NB))
        )

    def board_init(self) -> None:
        """Initialize the board's bitboards and other internal state.

        Must be called after creating a new board instance.
        """

        global _bb_initialized
        if not _bb_initialized:
            chess_lib.bb_init()
            _bb_initialized = True

        chess_lib.board_init(self.ptr)

    def zobrist_key(self) -> BitBoard:
        """Get the Zobrist hash key for the current position.

        The Zobrist key is a unique 64-bit hash of the current board state,
        useful for position comparison and transposition tables.

        Returns:
            BitBoard: 64-bit Zobrist hash value
        """
        hash = self.baseboard.hash
        return BitBoard(hash)

    def occ(self, color: Color) -> SquareSet:
        """Get a bitboard of all pieces of a given color.

        Args:
            color: WHITE (0), BLACK (1), or BOTH (2)

        Returns:
            SquareSet: Bitboard of all squares occupied by pieces of the given color

        Raises:
            ValueError: If color is not WHITE, BLACK, or BOTH
        """
        if not isinstance(color, Color) or color < WHITE or color > BOTH:
            raise ValueError(
                f"Invalid color value: {color}. Must be WHITE (0), BLACK (1) or BOTH (2)"
            )
        mask: BitBoard = self.baseboard.occ[color]
        return SquareSet(mask)

    def _get_piece_squares(self, piece: Piece, color: Color) -> SquareSet:
        if not isinstance(color, Color) or color < WHITE or color > BLACK:
            raise ValueError(
                f"Invalid color value: {color}. Must be WHITE (0), BLACK (1)"
            )
        piece_type = PieceType(piece, BLACK if color else WHITE)
        mask: BitBoard = self.baseboard.bb_squares[hash(piece_type)]
        return SquareSet(mask)

    def pawns(self, color: Color) -> SquareSet:
        return self._get_piece_squares(PAWN, color)

    def knights(self, color: Color) -> SquareSet:
        return self._get_piece_squares(KNIGHT, color)

    def rooks(self, color: Color) -> SquareSet:
        return self._get_piece_squares(ROOK, color)

    def bishops(self, color: Color) -> SquareSet:
        return self._get_piece_squares(BISHOP, color)

    def queens(self, color: Color) -> SquareSet:
        return self._get_piece_squares(QUEEN, color)

    def kings(self, color: Color) -> SquareSet:
        return self._get_piece_squares(KING, color)

    def king_sq(self, color: Color) -> Square:
        if not isinstance(color, Color) or color < WHITE or color > BOTH:
            raise ValueError(
                f"Invalid color value: {color}. Must be WHITE (0), BLACK (1) or BOTH (2)"
            )
        mask: BitBoard = self.kings(color).mask
        return utils.get_lsb(mask)

    def piece_at(self, square: Square) -> Optional[PieceType]:
        """Get the piece at a given square.

        Args:
            square: The square index (0-63)

        Returns:
            Optional[PieceType]: The piece on the square, or None if empty

        Raises:
            IndexError: If square is not in valid range 0-63
        """
        if not square in range(SQUARE_NB):
            raise IndexError
        piece = self.baseboard.squares[square]
        return PieceType.from_index(piece)

    def color_at(self, square: Square) -> Optional[Color]:
        """Get the color of the piece at a given square.

        Args:
            square: The square index (0-63)

        Returns:
            Optional[Color]: WHITE (0) or BLACK (1) if a piece is present,
                            None if the square is empty
        """
        mask: BitBoard = utils.bit(square)
        if self.occ(WHITE).mask & mask:
            return WHITE
        elif self.occ(BLACK).mask & mask:
            return BLACK
        else:
            return None

    def attacks_mask(self, square: Square) -> BitBoard:
        mask: BitBoard = chess_lib.bb_attacks_to_square(
            self.ptr, square, self.occ(BOTH).mask
        )
        return mask

    def attacks(self, color: Color, square: Square) -> SquareSet:
        """Get all pieces of a given color that attack a square.

        Args:
            color: The attacking color (WHITE or BLACK)
            square: The target square being attacked

        Returns:
            SquareSet: Bitboard of attacking pieces of the given color
        """
        mask: BitBoard = self.attacks_mask(square)
        return SquareSet(mask) & self.occ(color)

    def castling_rights(self) -> int:
        rights = self.baseboard.castle
        return int(rights)

    def clear(self) -> None:
        chess_lib.board_clear(self.ptr)

    def tolist(self) -> List[Optional[PieceType]]:
        arr: List[Optional[PieceType]] = [None] * SQUARE_NB
        for sq in self:
            arr[sq] = self.piece_at(sq)
        return arr

    def drawn_by_insufficient_material(self) -> bool:
        return bool(chess_lib.board_drawn_by_insufficient_material(self.ptr))

    def unicode_print(self) -> None:
        chess_lib.print_board(self.ptr)
        return None

    def board_to_fen(self) -> str:
        buffer: Array[c_char] = utils.create_string_buffer(256)
        chess_lib.board_to_fen(self.ptr, buffer)
        return buffer.value.decode("utf-8")

    def set_fen(self, fen: str) -> None:
        chess_lib.board_load_fen(self.ptr, fen.encode())

    def is_check(self) -> bool:
        return bool(chess_lib.is_check(self.ptr))

    def illegal_to_move(self) -> bool:
        return bool(chess_lib.illegal_to_move(self.ptr))

    def perft(self, depth: Optional[int] = 1) -> int:
        nodes = chess_lib.perft_test(self.ptr, depth)
        return int(nodes)

    def turn(self) -> Color:
        color = self.baseboard.color
        return Color(color)

    @property
    def ptr(self) -> object:
        return byref(self.baseboard)


class MoveUndo:
    def __init__(self, move: Move = Move.null()):
        self._move = move
        self.undo = Undo()

    @property
    def piece(self) -> Optional[PieceType]:
        """Get the piece that made the move.

        Returns:
            Optional[PieceType]: The piece type and color, or None if null move
        """
        if self._move:
            return PieceType.from_index(
                hash(self._move.piece) if self._move.piece else 0
            )
        return None

    @property
    def capture(self) -> Optional[PieceType]:
        """Get the piece that was captured by this move.

        Returns:
            Optional[PieceType]: The captured piece type and color, or None if no capture
        """
        capture = self.undo.capture
        return PieceType.from_index(capture)

    @property
    def enp_square(self) -> Optional[Square]:
        """Get the en passant square if this was an en passant capture.

        Returns:
            Optional[Square]: The square index (0-63) of the captured pawn in an en passant move,
                            or None if not an en passant capture
        """
        if self._move and self._move.is_enp():
            return utils.get_lsb(self.undo.ep)
        return None

    @property
    def ptr(self) -> Any:
        return byref(self.undo)

    def __repr__(self) -> str:
        return (
            f"{type(self).__name__}(san={self._move.san}, "
            f"piece={self.piece}, capture={self.capture}, "
            f"enp_square={self.enp_square})"
        )

    def __eq__(self, other: object) -> bool:
        if isinstance(other, MoveUndo):
            return self._move == other._move
        return False


class Board:
    """A chess board representation with move generation and validation.

    The Board class provides a high-level interface for chess game mechanics including:
    - Move generation and validation
    - Position tracking and manipulation
    - Game state checking (checkmate, stalemate, etc.)
    - FEN string parsing and generation
    - Board position display

    The board maintains the complete game state including:
    - Piece positions (via BaseBoard)
    - Side to move
    - Move history
    - Castling rights
    - En passant square
    - Move counters

    Attributes:
        board (BaseBoard): Low-level bitboard representation
        _handle_moves (MovesStack): Stack of moves and their undo information

    Example:
        >>> board = Board()  # Create new board with starting position
        >>> board.unicode()  # Display board with Unicode pieces
        >>> legal_moves = list(board.gen_legal_moves)  # Get legal moves
        >>> board.push(legal_moves[0])  # Make a move
        >>> board.pop()  # Take back the move
    """

    def __init__(self, fen: Optional[str] = None):
        self.board: BaseBoard = BaseBoard()
        if fen is None:
            self.set_fen(STARTING_FEN)
        else:
            self.set_fen(fen)
        self._handle_moves: MovesStack = MovesStack(self)

    def drawn_by_insufficient_material(self) -> bool:
        """Check if the position is drawn due to insufficient material.

        Returns:
            bool: True if neither side has sufficient material to checkmate
        """
        return self.board.drawn_by_insufficient_material()

    def clear(self) -> None:
        """Reset the board to starting position and clear move history."""
        self.board.clear()
        self._handle_moves.clear()
        self.set_fen(fen=STARTING_FEN)

    def castling_rights(self) -> int:

        return self.board.castling_rights()

    def _generate_moves(self, func_name: str) -> Iterator[Move]:
        array: Array[Any] = utils.create_uint32_array(MAX_MOVES)
        size: int = getattr(chess_lib, func_name)(self.board.ptr, array)
        for data in utils.scan_move_list(array[:size]):
            yield Move(*data)

    def generate_pseudo_legal_moves(self) -> Iterator[Move]:
        return self._generate_moves("gen_moves")

    def generate_legal_moves(self) -> Iterator[Move]:
        return self._generate_moves("gen_legal_moves")

    @property
    def turn(self) -> Color:
        """Get the turn color '0' for WHITE, '1' for BLACK"""
        return self.board.turn()

    @property
    def gen_legal_moves(self) -> LegalMoveGenerator:
        return LegalMoveGenerator(self)

    @property
    def gen_pseudo_legal_moves(self) -> PseudoLegalMoveGenerator:
        return PseudoLegalMoveGenerator(self)

    @property
    def fen(self) -> str:
        """Get FEN string representation of current position.

        Returns:
            str: The position in Forsyth–Edwards Notation
        """
        return self.board.board_to_fen()

    def set_fen(self, fen: str) -> None:
        """Set the board position from a FEN string.

        Args:
            fen: Position in Forsyth–Edwards Notation

        Raises:
            TypeError: If fen is not a string
            ValueError: If fen is empty or invalid
        """
        if not isinstance(fen, str):
            raise TypeError("FEN must be a string")
        if not fen:
            raise ValueError("FEN cannot be empty")
        self.board.set_fen(fen)

    def perft_test(self, depth: int = 1) -> BitBoard:
        """Calculate number of legal moves at given depth (performance test).

        Args:
            depth: Search depth (must be positive)

        Returns:
            BitBoard: Number of legal positions at given depth

        Raises:
            ValueError: If depth is not positive
        """
        if depth <= 0:
            raise ValueError("Depth must be non-negative")
        return self.board.perft(depth)

    def color_at(self, square: Square) -> Optional[Color]:
        """Get the color of the piece at given square.

        Args:
            square: Square index (0-63)

        Returns:
            Optional[Color]: WHITE, BLACK, or None if empty
        """
        return self.board.color_at(square)

    def is_game_over(self) -> bool:
        """Check if the game is over.

        Returns:
            bool: True if the game is over by any means
            (checkmate, stalemate, insufficient material, etc.)
        """
        return (
            self.is_checkmate()
            or self.is_stalemate()
            or self.drawn_by_insufficient_material()
            or self.is_fifty_moves()
            or self.is_threefold_repetition()
        )

    def is_checkmate(self) -> bool:
        """Check if the position is checkmate.

        Returns:
            bool: True if the side to move is in checkmate
        """
        return self.is_check() and not any(self.gen_legal_moves)

    def is_fifty_moves(self) -> bool:
        return False

    def is_threefold_repetition(self) -> bool:
        return False

    def is_stalemate(self) -> bool:
        """Check if the position is stalemate.

        Returns:
            bool: True if the side to move has no legal moves but is not in check
        """
        return not self.is_check() and not any(self.gen_legal_moves)

    def is_check(self) -> bool:
        """Check if the side to move is in check.

        Returns:
            bool: True if the current side's king is under attack
        """
        return self.board.is_check()

    def illegal_to_move(self) -> bool:
        return self.board.illegal_to_move()

    def checkers(self, color: Color) -> SquareSet:
        """Get pieces giving check to a king.

        Args:
            color: Color of the king being checked

        Returns:
            SquareSet: Bitboard of all pieces attacking the specified king

        Exemple:
        >>> board = Board("r1bqkb1r/pppp1Qpp/2n2n2/4p3/2B1P3/8/PPPP1PPP/RNB1K1NR b KQkq - 0 4")
        >>>
        >>> checkers = board.checkers(BLACK)
        >>> print(checkers)
        . . . . . . . .
        . . . . . 1 . .
        . . . . . . . .
        . . . . . . . .
        . . . . . . . .
        . . . . . . . .
        . . . . . . . .
        . . . . . . . .
        """
        king_sq: int = self.board.king_sq(color)
        return self.attackers(not color, king_sq)

    def is_square_attacked_by(self, color: Color, square: Square) -> bool:
        """Check if a square is attacked by any piece of the given color.

        Args:
            color: The attacking color (WHITE=0 or BLACK=1)
            square: The target square index (0-63) to check for attacks

        Returns:
            bool: True if the square is attacked by any piece of the specified color

        Raises:
            ValueError: If square is not a valid square index (0-63)

        Example:
            >>> board = Board("8/8/8/3k4/4Q3/8/8/3K4 w - - 0 1")
            >>> # Check if black king on d5 is attacked by white pieces
            >>> board.is_square_attacked_by(WHITE, D5)
            True
        """
        if not isinstance(square, int) or not (0 <= square < SQUARE_NB):
            raise ValueError(f"Invalid square index: {square}")
        return bool(self.board.attacks(color, square))

    def attackers(self, color: Color, square: Square) -> SquareSet:
        """Get all pieces of a given color that attack a specific square.

        This method returns a bitboard containing all pieces of the specified color
        that attack the given square in the current position.

        Args:
            color: The attacking color (WHITE=0 or BLACK=1)
            square: The target square index (0-63) being attacked

        Returns:
            SquareSet: Bitboard of all pieces of the given color that attack the square

        Raises:
            ValueError: If square is not a valid square index (0-63)

        Examples:
            >>> board = Board("8/8/8/3k4/4Q3/8/8/3K4 w - - 0 1")
            >>> attackers = board.attackers(WHITE, D5)  # Find white pieces attacking d5
            >>> print(attackers)  # Shows queen attacking black king
            . . . . . . . .
            . . . . . . . .
            . . . . . . . .
            . . . . . . . .
            . . . . 1 . . .
            . . . . . . . .
            . . . . . . . .
            . . . . . . . .
        """
        if not isinstance(square, int) or not (0 <= square < SQUARE_NB):
            raise ValueError(f"Invalid square index: {square}")
        return self.board.attacks(color, square)

    def push(self, move: Move) -> None:
        """Make a move on the board.

        Args:
            move: The move to make

        Raises:
            IllegalMoveError: If the move is not legal in current position

        Example:
        >>>
        >>> board = Board()
        >>> move = Move(E2, E3, PieceType(PAWN, WHITE))
        >>> board.push(move) # Make the move
        >>> board.pop() # Unmake the last move
        Move(san=e2e3, from=12, to=20, piece=PieceType'p', flag=0)
        """
        self._handle_moves.push(move)

    def pop(self) -> Move:
        """Take back the last move made.

        Returns:
            Move: The move that was undone

        Raises:
            IndexError: If there are no moves to undo
        """
        return self._handle_moves.pop()

    def push_null(self) -> None:
        """Push a null move this will just flip the color
        and update the zobrist hash.
        """
        return None

    def pop_null(self) -> Move:
        return NotImplemented

    def peek(self) -> Optional[Move]:
        return self._handle_moves.peek()

    def copy(self) -> Self:
        dst = type(self)(None)
        pointer(dst.board.baseboard)[
            0
        ] = (
            self.board.baseboard
        )  # https://stackoverflow.com/questions/1470343/python-copying-structures-contents

        dst._handle_moves = self._handle_moves.copy(dst)
        return dst

    def __copy__(self) -> Self:
        return self.copy()

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, Board):
            return NotImplemented
        return (
            self.fen == other.fen
            and self._handle_moves == other._handle_moves
            and self.board == other.board
        )

    def __repr__(self) -> str:
        return f"{type(self).__name__}(fen={self.fen!r})"

    def __str__(self) -> str:
        """
        Returns a string representation of the board with ranks and files.

        >>> import sisyphus
        >>>
        >>> board = sisyphus.Board()
        >>> print (board)

        8 R N B Q K B N R
        7 P P P P P P P P
        6 . . . . . . . .
        5 . . . . . . . .
        4 . . . . . . . .
        3 . . . . . . . .
        2 p p p p p p p p
        1 r n b q k b n r
        a b c d e f g h

        """
        builder: List[str] = []

        for rank in range(7, -1, -1):
            builder.append(f"{rank + 1} ")

            for file in range(8):
                square = utils.square(rank, file)
                piece = self.board.piece_at(square)

                if piece:
                    builder.append(piece.symbol())
                else:
                    builder.append(".")
                builder.append(" ")

            builder.append("\n")

        builder.append("  a b c d e f g h")

        return "".join(builder)

    def unicode(self) -> None:
        """
        pretty-printing the current board position using Unicode chess symbols.

        >>>
        >>> board = sisyphus.Board()
        >>> board.unicode()

        8 ♖ ♘ ♗ ♕ ♔ ♗ ♘ ♖
        7 ♙ ♙ ♙ ♙ ♙ ♙ ♙ ♙
        6 . . . . . . . .
        5 . . . . . . . .
        4 . . . . . . . .
        3 . . . . . . . .
        2 ♟ ♟ ♟ ♟ ♟ ♟ ♟ ♟
        1 ♜ ♞ ♝ ♛ ♚ ♝ ♞ ♜
        a b c d e f g h


        rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -
        """
        self.board.unicode_print()

    def __hash__(self) -> int:
        return self.board.zobrist_key()


class PseudoLegalMoveGenerator:
    """Generator for pseudo legal moves in a given position."""

    def __init__(self, board: Board):
        self.board = board

    def __bool__(self) -> bool:
        return any(self.board.generate_pseudo_legal_moves())

    def __len__(self) -> int:
        return len(list(self))

    def __iter__(self) -> Iterator[Move]:
        return self.board.generate_pseudo_legal_moves()

    def __contains__(self, move: Move) -> bool:
        return move in set(
            self
        )  # TODO: add a function that check if a move is psaudo legal

    def __repr__(self) -> str:
        sans = ", ".join(m.san for m in self)
        return f"{type(self).__name__} at {id(self):#x} ({sans})"


class LegalMoveGenerator:
    """Generator for legal moves in a given position."""

    def __init__(self, board: Board):
        self.board = board

    def __bool__(self) -> bool:
        return any(self.board.generate_legal_moves())

    def __iter__(self) -> Iterator[Move]:
        return self.board.generate_legal_moves()

    def __contains__(self, move: Move) -> bool:
        return move in set(
            self
        )  # TODO: add a function that check if a move is a legal move

    def __repr__(self) -> str:
        sans = ", ".join(m.san for m in self)
        return f"{type(self).__name__} at {id(self):#x} ({sans})"


class MovesStack:
    """A simple stack data structure that keeps track of chess moves and their undo information."""

    def __init__(self, board: Board) -> None:
        self.board = board
        self._moves_history: List[Tuple[Move, MoveUndo]] = []

    def push(self, move: object) -> None:
        if not isinstance(move, Move):
            raise TypeError(f"Expected Move instance, got {type(move).__name__}")

        # NOTE: we don' t check for move legality.
        # if not move in LegalMoveGenerator(self.board):
        #    raise IllegalMoveError(
        #        f"Move {move.san} is not legal in the current position."
        #    )

        undo: MoveUndo = MoveUndo(move)
        chess_lib.do_move(self.board.board.ptr, hash(move), undo.ptr)
        self._moves_history.append((move, undo))

    def pop(self) -> Move:
        if len(self):
            move, undo = self._moves_history.pop()
        else:
            raise IndexError("pop from empty move history")

        chess_lib.undo_move(self.board.board.ptr, hash(move), undo.ptr)
        return move

    def peek(self) -> Optional[Move]:
        return self._moves_history[-1][0] if self._moves_history else None

    def clear(self) -> None:
        self._moves_history.clear()

    def copy(self, other: Board) -> Self:
        dst = type(self)(other)
        dst._moves_history = self._moves_history.copy()
        return dst

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, MovesStack):
            return NotImplemented
        return self._moves_history == other._moves_history

    def __len__(self) -> int:
        return len(self._moves_history)

    def __iter__(self) -> Iterator[Tuple[Move, MoveUndo]]:
        return ((mv, undo) for mv, undo in self._moves_history)

    def __contains__(self, move: Move) -> bool:
        return any(mv == move for mv, _ in self)

    def __repr__(self) -> str:
        moves = ", ".join(mv.san for mv, _ in self)
        return f"<{type(self).__name__} id={id(self):#x} moves=[{moves}]>"
