import logging
import sys
import unittest
import sisyphus


class RaiseLogHandler(logging.StreamHandler):
    def handle(self, record):
        super().handle(record)
        raise RuntimeError("was expecting no log messages")


class UtilsTestCase(unittest.TestCase):
    def test_square(self):
        for square in list(range(sisyphus.SQUARE_NB)):
            file_sq = sisyphus.utils.file_of(square)
            rank_of = sisyphus.utils.rank_of(square)
            self.assertEqual(sisyphus.utils.square(rank_of, file_sq), square)

    def test_bit_operations(self):
        bb = sisyphus.utils.bit(sisyphus.C2)
        self.assertEqual(sisyphus.utils.get_lsb(bb), 10)
        self.assertEqual(sisyphus.utils.popcount(bb), 1)
        self.assertTrue(sisyphus.utils.test_bit(bb, 10))
        self.assertFalse(sisyphus.utils.test_bit(bb, 11))

    def test_square_distances(self):
        self.assertEqual(sisyphus.utils.square_distance(sisyphus.A1, sisyphus.H8), 7)
        self.assertEqual(sisyphus.utils.square_distance(sisyphus.E4, sisyphus.E5), 1)
        self.assertEqual(
            sisyphus.utils.square_manhattan_distance(sisyphus.A1, sisyphus.H8), 14
        )
        self.assertEqual(
            sisyphus.utils.square_manhattan_distance(sisyphus.E4, sisyphus.E5), 1
        )

    def test_scan_forward(self):
        bb = (1 << sisyphus.A1) | (1 << sisyphus.C3) | (1 << sisyphus.H8)
        squares = list(sisyphus.utils.scan_forward(bb))
        self.assertEqual(squares, [sisyphus.A1, sisyphus.C3, sisyphus.H8])

    def test_file_and_rank(self):
        self.assertEqual(sisyphus.utils.file_of(sisyphus.E4), 4)
        self.assertEqual(sisyphus.utils.rank_of(sisyphus.E4), 3)
        self.assertEqual(sisyphus.utils.file_of(sisyphus.H8), 7)
        self.assertEqual(sisyphus.utils.rank_of(sisyphus.H8), 7)


class BoardTestCase(unittest.TestCase):
    def test_default_position(self):
        board = sisyphus.Board()
        self.assertEqual(board.turn, sisyphus.WHITE)
        self.assertEqual(board.fen, sisyphus.STARTING_FEN)
        self.assertEqual(board.turn, sisyphus.WHITE)

    def test_eq(self):
        b1 = sisyphus.Board(
            "r1bqkb1r/ppppppp1/2n2n1p/6B1/3P4/2N5/PPP1PPPP/R2QKBNR w KQkq - 0 1"
        )
        b2 = sisyphus.Board(
            "r1bqkb1r/ppppppp1/2n2n1p/6B1/3P4/2N5/PPP1PPPP/R2QKBNR w KQkq - 0 1"
        )
        self.assertEqual(b1, b2)

    def test_fen(self):
        test_positions = [
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -",
            "rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq -",
            "r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq -",
            "8/8/8/8/8/8/8/8 w - -",
        ]

        board = sisyphus.Board()

        for fen in test_positions:
            board.set_fen(fen)
            self.assertEqual(board.fen, fen)

        with self.assertRaises(ValueError):
            board.set_fen("")

        with self.assertRaises(TypeError):
            board.set_fen(None)

    def test_move_making(self):
        board = sisyphus.Board()
        count = 0
        for move in board.gen_legal_moves:
            board.push(move)
            for move in board.gen_legal_moves:
                board.push(move)
                count += 1
                board.pop()
            board.pop()
        self.assertEqual(count, 400)
        with self.assertRaises(TypeError):
            board.push(None)

    def test_color_at(self):
        board = sisyphus.Board()
        self.assertEqual(board.color_at(sisyphus.A1), sisyphus.WHITE)
        self.assertEqual(board.color_at(sisyphus.G7), sisyphus.BLACK)
        self.assertEqual(board.color_at(sisyphus.E4), None)

    def test_clear(self):
        board = sisyphus.Board()
        moves = board.gen_legal_moves
        move = sisyphus.Move(
            sisyphus.A2, sisyphus.A3, sisyphus.PieceType(sisyphus.PAWN, sisyphus.WHITE)
        )
        board.push(move)
        board.clear()
        self.assertEqual(board.fen, sisyphus.STARTING_FEN)
        self.assertEqual(len(board._handle_moves), 0)

    def test_perft(self):
        test_positions = {
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1": [
                (1, 20),
                (2, 400),
                (3, 8902),
                (4, 197281),
                (5, 4865609),
                (6, 119060324),
            ],
            "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ": [
                (1, 48),
                (2, 2039),
                (3, 97862),
                (4, 4085603),
                (5, 193690690),
            ],
            "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1": [
                (1, 6),
                (2, 264),
                (3, 9467),
                (4, 422333),
                (5, 15833292),
            ],
            "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ": [
                (1, 14),
                (2, 191),
                (3, 2812),
                (4, 43238),
                (5, 674624),
                (6, 11030083),
                (7, 178633661),
            ],
            "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8": [
                (1, 44),
                (2, 1486),
                (3, 62379),
                (4, 2103487),
            ],
            "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10": [
                (1, 46),
                (2, 2079),
                (3, 89890),
                (4, 3894594),
                (5, 164075551),
            ],
        }

        board = sisyphus.Board()

        max_test_depth = 3
        for fen, expected_results in test_positions.items():
            board.set_fen(fen)

            for depth, expected_nodes in expected_results:
                if depth <= max_test_depth:
                    nodes = board.perft_test(depth)
                    self.assertEqual(
                        nodes,
                        expected_nodes,
                        f"Perft failed at depth {depth} for position {fen}\n"
                        f"Expected {expected_nodes} nodes, got {nodes}",
                    )

        with self.assertRaises(ValueError):
            board.perft_test(0)
        with self.assertRaises(ValueError):
            board.perft_test(-1)

    def test_attackers(self):
        board = sisyphus.Board(
            "r1b1k2r/pp1n1ppp/2p1p3/q5B1/1b1P4/P1n1PN2/1P1Q1PPP/2R1KB1R b Kkq - 3 10"
        )

        attackers = board.attackers(sisyphus.WHITE, sisyphus.C3)
        self.assertEqual(len(attackers), 3)
        self.assertIn(sisyphus.C1, attackers)
        self.assertIn(sisyphus.D2, attackers)
        self.assertIn(sisyphus.B2, attackers)
        self.assertNotIn(sisyphus.D4, attackers)
        self.assertNotIn(sisyphus.E1, attackers)

    def test_check(self):
        board = sisyphus.Board(
            "rnbqkbnr/ppp2ppp/3p4/1B2Q3/8/8/PPPPPPPP/RN2KBNR b KQkq - 0 1"
        )
        self.assertTrue(board.is_check())

    def test_checkers(self):
        board = sisyphus.Board(
            "rnbqkbnr/ppp2ppp/3p4/1B2Q3/8/8/PPPPPPPP/RN2KBNR b KQkq - 0 1"
        )
        mask = board.checkers(sisyphus.BLACK)
        self.assertEqual(len(mask), 2)

    def test_checkmate(self):
        board = sisyphus.Board()
        self.assertFalse(board.is_checkmate())
        board.set_fen("rnbqkbnr/ppp2ppp/3p4/1B2Q3/8/8/PPPPPPPP/RN2KBNR b KQkq - 0 1")
        self.assertTrue(board.is_checkmate())

    def test_castling_rights(self):
        board = sisyphus.Board()
        self.assertEqual(board.castling_rights(), sisyphus.CASTLE_ALL)

        board = sisyphus.Board("1q1k4/2Rr4/8/2Q3K1/8/8/8/8 w - - 0 1")
        self.assertEqual(board.castling_rights(), sisyphus.EMPTY_FLAG)


    def test_peek(self):
        board = sisyphus.Board()
        move = sisyphus.Move(
            sisyphus.A2, sisyphus.A3, sisyphus.PieceType(sisyphus.PAWN, sisyphus.WHITE)
        )
        board.push(move)
        self.assertEqual(board.peek(), move)
        board.pop()

        self.assertEqual(board.peek(), None)


class BaseBoardTestCase(unittest.TestCase):
    def test_equal(self):
        a1 = sisyphus.BaseBoard()
        a2 = sisyphus.BaseBoard()
        self.assertEqual(a1, a2)

    def test_init(self):
        board = sisyphus.BaseBoard()

        for c in range(sisyphus.BOTH):
            self.assertEqual(len(board.pawns(c)), 8)
            self.assertEqual(len(board.knights(c)), 2)
            self.assertEqual(len(board.bishops(c)), 2)
            self.assertEqual(len(board.rooks(c)), 2)
            self.assertEqual(len(board.queens(c)), 1)
            self.assertEqual(len(board.kings(c)), 1)

    def test_king_sq(self):
        board = sisyphus.BaseBoard()
        self.assertEqual(board.king_sq(sisyphus.WHITE), sisyphus.E1)
        self.assertEqual(board.king_sq(sisyphus.BLACK), sisyphus.E8)

    def test_piece_at(self):
        board = sisyphus.BaseBoard()
        self.assertEqual(board.piece_at(sisyphus.D1), sisyphus.PieceType.from_symbol("q"))
        self.assertEqual(board.piece_at(sisyphus.F8), sisyphus.PieceType.from_symbol("B"))
        self.assertEqual(board.piece_at(sisyphus.D3), None)

        with self.assertRaises(IndexError):
            board.piece_at(-1)

    def test_color_at(self):
        board = sisyphus.BaseBoard()
        self.assertEqual(board.color_at(sisyphus.A2), sisyphus.WHITE)
        self.assertEqual(board.color_at(sisyphus.A7), sisyphus.BLACK)


class SquareSetTestCase(unittest.TestCase):
    def test_len(self):
        mask = sisyphus.utils.bit(sisyphus.A1) | sisyphus.utils.bit(sisyphus.B1)
        a1 = sisyphus.SquareSet(mask)
        self.assertTrue(bool(a1))
        self.assertEqual(len(a1), 2)

    def test_contains(self):
        mask = sisyphus.utils.bit(sisyphus.A5)
        a1 = sisyphus.SquareSet(mask)

        self.assertTrue(sisyphus.A5 in a1)
        self.assertFalse(sisyphus.B1 in a1)

    def test_pop(self):
        mask = sisyphus.utils.bit(sisyphus.A5)
        a1 = sisyphus.SquareSet(mask)

        self.assertEqual(a1.pop(), sisyphus.A5)
        with self.assertRaises(KeyError):
            a1.pop()


class MoveTestCase(unittest.TestCase):
    def test_default(self):
        board = sisyphus.Board()
        m1 = sisyphus.Move(
            sisyphus.A2, sisyphus.A3, sisyphus.PieceType(sisyphus.PAWN, sisyphus.WHITE)
        )
        self.assertIn(m1, board.gen_legal_moves)

        board.push(m1)  # Do move
        board.pop()  # Undo move

    def test_parse_uci(self):
        board = sisyphus.Board()
        m1 = sisyphus.Move.parse_uci(board, "g1f3")
        self.assertIn(m1, board.gen_legal_moves)

        with self.assertRaises(ValueError):
            sisyphus.Move.parse_uci(board, "")

    def test_san(self):
        move = sisyphus.Move(
            sisyphus.G1, sisyphus.F3, sisyphus.PieceType(sisyphus.KNIGHT, sisyphus.WHITE)
        )
        self.assertEqual(move.san, "g1f3")


class PiecTypeTestCase(unittest.TestCase):
    def test_symbol(self):
        p1 = sisyphus.PieceType(sisyphus.PAWN, sisyphus.WHITE)
        p2 = sisyphus.PieceType(sisyphus.KING, sisyphus.WHITE)
        p3 = sisyphus.PieceType(sisyphus.QUEEN, sisyphus.BLACK)

        self.assertEqual(p1.symbol(), "P")
        self.assertEqual(p2.symbol(), "K")
        self.assertEqual(p3.symbol(), "q")

    def test_to_index(self):
        piece = sisyphus.PieceType(sisyphus.QUEEN, sisyphus.BLACK)
        self.assertEqual(hash(piece), 9)

    def test_from_index(self):
        piece = sisyphus.PieceType(sisyphus.QUEEN, sisyphus.BLACK)
        self.assertEqual(sisyphus.PieceType.from_index(9), piece)

    def test_from_symbol(self):
        piece = sisyphus.PieceType(sisyphus.QUEEN, sisyphus.BLACK)
        self.assertEqual(sisyphus.PieceType.from_symbol("Q"), piece)


if __name__ == "__main__":
    verbosity = sum(
        arg.count("v") for arg in sys.argv if all(c == "v" for c in arg.lstrip("-"))
    )
    verbosity += sys.argv.count("--verbose")

    if verbosity >= 2:
        logging.basicConfig(level=logging.DEBUG)

    raise_log_handler = RaiseLogHandler()
    raise_log_handler.setLevel(logging.ERROR)
    logging.getLogger().addHandler(raise_log_handler)

    unittest.main()
