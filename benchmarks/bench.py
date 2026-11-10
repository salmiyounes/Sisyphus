"""Benchmark Sisyphus against python-chess.

Usage:
    python benchmarks/bench.py            # default run
    python benchmarks/bench.py --deep     # also run Kiwipete at depth 4 (slower)
    python benchmarks/bench.py --repeat 9 # more repeats for steadier medians

Sections:
    1. Python-driven perft   (fair: same Python loop for both libraries)
    2. Move generation only  (C call + Move object creation)
    3. push/pop throughput
    4. copy() cost, plus a breakdown of where Sisyphus spends its time
    5. Native perft          (C only: engine ceiling, not a library comparison)

Node counts are asserted before timings are trusted.
"""

from __future__ import annotations

import argparse
import platform
import statistics
import sys
import time
import timeit

import chess
import sisyphus

START_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
KIWIPETE_FEN = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"

# (name, fen, depth, expected nodes)
PERFT_CASES = [
    ("startpos", START_FEN, 4, 197_281),
    ("kiwipete", KIWIPETE_FEN, 3, 97_862),
]
PERFT_DEEP_CASES = [
    ("kiwipete", KIWIPETE_FEN, 4, 4_085_603),
]

# Known perft counts for the native test (startpos)
NATIVE_PERFT_DEPTH = 5
NATIVE_PERFT_EXPECTED = 4_865_609


# --------------------------------------------------------------------------
# Helpers
# --------------------------------------------------------------------------
def timed(fn, repeat: int, warmup: int = 1):
    """Run fn `warmup` times (ignored), then `repeat` times.

    Returns (median_seconds, min_seconds, last_result).
    """
    result = None
    for _ in range(warmup):
        result = fn()
    runs = []
    for _ in range(repeat):
        t0 = time.perf_counter()
        result = fn()
        runs.append(time.perf_counter() - t0)
    return statistics.median(runs), min(runs), result


def report(name: str, t_chess: float, t_sis: float) -> None:
    ratio = t_chess / t_sis if t_sis else float("inf")
    print(
        f"{name:<34} chess={t_chess:9.4f}s  sisyphus={t_sis:9.4f}s  "
        f"ratio={ratio:6.2f}x"
    )


def header(title: str) -> None:
    print(f"\n=== {title} ===")


def print_environment() -> None:
    print("Environment")
    print(f"  python        : {sys.version.split()[0]}")
    print(f"  platform      : {platform.platform()}")
    print(f"  processor     : {platform.processor() or platform.machine()}")
    print(f"  python-chess  : {getattr(chess, '__version__', 'unknown')}")
    print(f"  sisyphus      : {getattr(sisyphus, '__version__', 'unknown')}")
    print("  ratio = chess_time / sisyphus_time  (>1 means Sisyphus is faster)")


# --------------------------------------------------------------------------
# Perft implementations (same algorithm for both libraries)
# --------------------------------------------------------------------------
def perft_chess(board: chess.Board, depth: int) -> int:
    if depth == 1:
        return board.legal_moves.count()
    nodes = 0
    for move in list(board.legal_moves):
        board.push(move)
        nodes += perft_chess(board, depth - 1)
        board.pop()
    return nodes


def sis_moves(board):
    # README shows a `gen_legal_moves` property; the module source exposes
    # a `generate_legal_moves()` method. Support both.
    gen = getattr(board, "generate_legal_moves", None)
    if callable(gen):
        return list(gen())
    return list(board.gen_legal_moves)


def perft_sis(board, depth: int) -> int:
    moves = sis_moves(board)
    if depth == 1:
        return len(moves)
    nodes = 0
    for move in moves:
        board.push(move)
        nodes += perft_sis(board, depth - 1)
        board.pop()
    return nodes


# --------------------------------------------------------------------------
# Benchmarks
# --------------------------------------------------------------------------
def bench_python_perft(cases, repeat: int) -> None:
    header("1. Python-driven perft (fair comparison)")
    for name, fen, depth, expected in cases:
        cb, sb = chess.Board(fen), sisyphus.Board(fen)

        n_c = perft_chess(cb, depth)
        n_s = perft_sis(sb, depth)
        assert n_c == n_s == expected, (
            f"node count mismatch on {name} d{depth}: "
            f"chess={n_c} sisyphus={n_s} expected={expected}"
        )

        t_c, _, _ = timed(lambda: perft_chess(cb, depth), repeat)
        t_s, _, _ = timed(lambda: perft_sis(sb, depth), repeat)
        report(f"perft {name} d{depth} ({expected} nodes)", t_c, t_s)


def bench_movegen(repeat: int) -> None:
    header("2. Legal move generation only")
    n = 20_000
    for name, fen in (("startpos", START_FEN), ("kiwipete", KIWIPETE_FEN)):
        cb, sb = chess.Board(fen), sisyphus.Board(fen)
        assert len(list(cb.legal_moves)) == len(sis_moves(sb))

        def run_chess():
            for _ in range(n):
                list(cb.legal_moves)

        def run_sis():
            for _ in range(n):
                sis_moves(sb)

        t_c, _, _ = timed(run_chess, repeat)
        t_s, _, _ = timed(run_sis, repeat)
        report(f"movegen {name} x{n}", t_c, t_s)


def bench_push_pop(repeat: int) -> None:
    header("3. push/pop throughput")
    n = 2_000
    cb, sb = chess.Board(KIWIPETE_FEN), sisyphus.Board(KIWIPETE_FEN)
    c_moves = list(cb.legal_moves)
    s_moves = sis_moves(sb)
    assert len(c_moves) == len(s_moves)

    def run_chess():
        for _ in range(n):
            for m in c_moves:
                cb.push(m)
                cb.pop()

    def run_sis():
        for _ in range(n):
            for m in s_moves:
                sb.push(m)
                sb.pop()

    t_c, _, _ = timed(run_chess, repeat)
    t_s, _, _ = timed(run_sis, repeat)
    report(f"push+pop x{n * len(c_moves)}", t_c, t_s)


def bench_copy(repeat: int) -> None:
    header("4. copy() cost")
    n = 10_000
    cb, sb = chess.Board(KIWIPETE_FEN), sisyphus.Board(KIWIPETE_FEN)

    t_c, _, _ = timed(lambda: [cb.copy() for _ in range(n)], repeat)
    t_s, _, _ = timed(lambda: [sb.copy() for _ in range(n)], repeat)
    report(f"copy x{n}", t_c, t_s)
    print(f"  per copy: chess={t_c / n * 1e6:.2f}us  sisyphus={t_s / n * 1e6:.2f}us")

    # Sanity: the copy must be an independent, equal position.
    cp = sb.copy()
    assert len(sis_moves(cp)) == len(sis_moves(sb)), "copy() changed the position"
    if hasattr(sb, "fen"):
        assert cp.fen == sb.fen, "copy() produced a different position"

    # Breakdown of Sisyphus copy(): mirrors the steps in Board.copy()
    #     dst = type(self)(default=False)
    #     chess_lib.board_copy(dst.ptr, self.ptr)
    #     dst._handle_moves = self._handle_moves.copy(dst)
    # and splits Board.__init__ into its parts. Each step is skipped if the
    # internal name it needs is not exported by the sisyphus package.
    print("  Sisyphus copy() breakdown (per call):")

    def step(label, fn):
        try:
            total = timeit.timeit(fn, number=n)
            print(f"    {label:<28}: {total / n * 1e6:8.2f}us")
        except Exception as exc:  # noqa: BLE001
            print(f"    {label:<28}: skipped ({exc!r})")

    cls = type(sb)
    dst = cls(default=False)

    # Whole thing, then its pieces
    step("full copy()", sb.copy)
    step("construct Board(default=False)", lambda: cls(default=False))

    # Parts of Board.__init__
    base_cls = getattr(sisyphus, "BaseBoard", None)
    if base_cls is not None:
        step(
            "  BaseBoard.__init__ only", lambda: base_cls.__init__(object.__new__(cls))
        )
    stack_cls = getattr(sisyphus, "MovesStack", None)
    if stack_cls is not None:
        step("  MovesStack(board)", lambda: stack_cls(sb))
    make_buf = getattr(sisyphus, "create_uint32_array", None)
    max_moves = getattr(sisyphus, "MAX_MOVES", None)
    if make_buf is not None and max_moves is not None:
        step("  create_uint32_array(MAX)", lambda: make_buf(max_moves))

    # Parts of copy() after construction
    lib = getattr(sisyphus, "chess_lib", None)
    if lib is not None:
        step("C board_copy (ctypes call)", lambda: lib.board_copy(dst.ptr, sb.ptr))
    step("move-stack copy", lambda: sb._handle_moves.copy(dst))


def bench_native_perft(repeat: int) -> None:
    header("5. Native perft (C only: engine ceiling)")
    print(f"  startpos depth {NATIVE_PERFT_DEPTH}, fresh Board() for every run")
    rates = []
    for i in range(max(repeat, 3)):
        board = sisyphus.Board()
        t0 = time.perf_counter()
        nodes = board.perft_test(NATIVE_PERFT_DEPTH)
        dt = time.perf_counter() - t0
        assert (
            nodes == NATIVE_PERFT_EXPECTED
        ), f"native perft mismatch: got {nodes}, expected {NATIVE_PERFT_EXPECTED}"
        rate = nodes / dt / 1e6 if dt else float("inf")
        rates.append(rate)
        print(f"  run {i + 1}: {nodes} nodes in {dt:.4f}s ({rate:.1f}M nodes/s)")
    print(f"  median: {statistics.median(rates):.1f}M nodes/s")
    if max(rates) > 20 * min(rates):
        print(
            "  WARNING: runs differ wildly. perft_test may be caching results "
            "between calls, so only the first run is a real measurement."
        )


# --------------------------------------------------------------------------
def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument(
        "--repeat", type=int, default=5, help="timed repeats (default 5)"
    )
    parser.add_argument("--deep", action="store_true", help="also run Kiwipete depth 4")
    args = parser.parse_args()

    print_environment()

    cases = list(PERFT_CASES)
    if args.deep:
        cases += PERFT_DEEP_CASES

    bench_python_perft(cases, args.repeat)
    bench_movegen(args.repeat)
    bench_push_pop(args.repeat)
    bench_copy(args.repeat)
    bench_native_perft(args.repeat)


if __name__ == "__main__":
    main()
