#!/usr/bin/env python3
from __future__ import annotations

import sys
from typing import Any, Dict, List

import sisyphus
from sisyphus import Board, Move, Searcher


class UCI:
    """UCI protocol implementation for chess engine."""

    def __init__(self):
        self.board = Board()
        self.searcher = Searcher(self.board)
        self.count = 0
        self.debug = False

    def print_banner(self) -> None:
        """Print engine banner with name and version."""
        banner = """
    ███████ ██ ███████ ██    ██ ██████  ██   ██ ██    ██ ███████ 
    ██      ██ ██       ██  ██  ██   ██ ██   ██ ██    ██ ██      
    ███████ ██ ███████   ████   ██████  ███████ ██    ██ ███████ 
        ██ ██      ██    ██    ██      ██   ██ ██    ██      ██ 
    ███████ ██ ███████    ██    ██      ██   ██  ██████  ███████ 
    """
        print(banner)
        print(f"\t\t\t\t\t\tAuthor: {sisyphus.__author__}")
        print(f"\t\t\t\t\t\tVersion: {sisyphus.__version__}\n")

    def _position(self, args: List[str]) -> None:
        """Handle 'position' command."""
        if not args:
            return

        if args[0] == "startpos":
            self.board.clear()
            args = args[1:]
        elif args[0] == "fen":
            fen = " ".join(args[1:7])
            self.board.set_fen(fen)
            args = args[7:]

        if args and args[0] == "moves":
            for move in args[1:]:
                self.board.push(Move.parse_uci(self.board, move))
                self.count += 1

    def _go(self, args: List[str]) -> None:
        """Handle 'go' command."""
        params: Dict[str, Any] = {}

        i = 0
        while i < len(args):
            param = args[i]

            if param in [
                "depth",
                "movetime",
                "movestogo",
                "wtime",
                "winc",
                "binc",
                "btime",
            ]:
                i += 1
                if i < len(args):
                    params[param] = int(args[i])
            elif param in ["infinite", "ponder"]:
                params[param] = True

            i += 1

        # Start search with parameters
        if "movetime" in params:
            movetime = params["movetime"] / 1000
            best_move = self.searcher.start(time_s=movetime)

        elif "wtime" in params:
            wtime = params.get("wtime", 0) / 1000
            btime = params.get("btime", 0) / 1000
            winc = params.get("winc", 0) / 1000
            binc = params.get("binc", 0) / 1000

            if not self.board.turn:
                time, inc = wtime, winc
            else:
                time, inc = btime, binc

            think = min(time / 40 + inc, time / 2 - 1)
            if self.count < 3:
                think = min(think, 1.0)

            best_move = self.searcher.start(time_s=think)

        elif "depth" in params:
            best_move = self.searcher.start(depth=params["depth"])
        else:
            best_move = self.searcher.start()

        self.count = 0
        print(f"bestmove {best_move.move_str()}")

    def loop(self) -> None:
        """Main UCI command loop."""

        self.print_banner()

        while True:
            try:
                command = input().strip()

                if not command:
                    continue

                tokens = command.split()
                cmd, args = tokens[0], tokens[1:]

                if cmd == "quit":
                    break

                elif cmd == "stop":
                    self.searcher.stop()

                elif cmd == "uci":
                    print("id name Sisyphus")
                    print(f"id author {sisyphus.__author__}")
                    print("uciok")

                elif cmd == "isready":
                    print("readyok")

                elif cmd == "ucinewgame":
                    self.board.clear()
                    self.searcher.clear()

                elif cmd == "position":
                    self._position(args)

                elif cmd == "go":
                    self._go(args)

                elif cmd == "debug":
                    self.debug = args[0] == "on" if args else not self.debug

            except Exception as e:
                print(f"info string Error: {e}", file=sys.stderr)


def main() -> None:
    """Entry point for UCI interface."""
    uci = UCI()
    uci.loop()


if __name__ == "__main__":
    main()
