<div align="center">

# Sisyphus

![Lichess bullet ranking](https://lichess-shield.vercel.app/api?username=Sisyphus-engine&format=bullet)
![Lichess blitz ranking](https://lichess-shield.vercel.app/api?username=Sisyphus-engine&format=blitz)
![Lichess rapid ranking](https://lichess-shield.vercel.app/api?username=Sisyphus-engine&format=rapid)
</div>

Introduction
------------

Sisyphus is a chess engine and library that combines the raw performance of **C** with the flexibility and ease of use of **Python**.  
It features a simple implementation of the `Universal Chess Interface (UCI) <https://en.wikipedia.org/wiki/Universal_Chess_Interface>`_ protocol, enabling seamless integration with popular chess GUIs.  
In addition to UCI support, Sisyphus provides efficient move generation, move validation, and other essential chess functionalities.

Features
--------

- **Board Representation**

  - `Bitboards <https://www.chessprogramming.org/Bitboards>`_
  - `Magic Bitboards <https://www.chessprogramming.org/Magic_Bitboards>`_

- **Search Algorithms**

  - `Negamax <https://www.chessprogramming.org/Negamax>`_
  - `Principal Variation Search (PVS) <https://www.chessprogramming.org/Principal_Variation_Search>`_
  - `Transposition Table <https://www.chessprogramming.org/Transposition_Table>`_
  - `Late Move Reductions (LMR) <https://www.chessprogramming.org/Late_Move_Reductions>`_
  - `Null Move Pruning <https://www.chessprogramming.org/Null_Move_Pruning>`_
  - `Razoring <https://www.chessprogramming.org/Razoring>`_
  - `Delta Pruning <https://www.chessprogramming.org/Delta_Pruning>`_
  - `Quiescence Search <https://www.chessprogramming.org/Quiescence_Search>`_
  - `Internal Iterative Reductions <https://www.chessprogramming.org/Reductions>`_
  - `Reverse Futility Pruning <https://www.chessprogramming.org/Reverse_Futility_Pruning>`_
  - `Aspiration Window <https://www.chessprogramming.org/Aspiration_Window>`_

- **Move Ordering Techniques**

  - `History Heuristic <https://www.chessprogramming.org/History_Heuristic>`_
  - `Killer Heuristic <https://www.chessprogramming.org/Killer_Heuristic>`_
  - `MVV-LVA (Most Valuable Victim - Least Valuable Attacker) <https://www.chessprogramming.org/MVV-LVA>`_
  - `Static Exchange Evaluation (SEE) <https://www.chessprogramming.org/Static_Exchange_Evaluation>`_

- **Evaluation**

  - `Piece-Square Tables (PSTs) <https://www.chessprogramming.org/Piece-Square_Tables>`_
  - `PeSTO Evaluation Framework <https://www.chessprogramming.org/PeSTO>`_

Installing
----------

Prerequisites
~~~~~~~~~~~~

- Python 3.11+
- Git
- pip (Python package installer)
- A C compiler (gcc or clang)

Installation Steps
~~~~~~~~~~~~~~~~

1. Clone the repository::

    git clone https://github.com/salmiyounes/Sisyphus.git

2. Navigate to the project directory::

    cd Sisyphus/

3. Install the package::

    pip install .

Use Sisyphus as library
-----------------------
* Move generation

    .. code:: python 

        >>> import sisyphus

        >>> board = sisyphus.Board()
        >>> board.gen_legal_moves
        LegalMoveGenerator at 0x7e1c20527fb0 (a2a3, b2b3, c2c3, d2d3, e2e3, f2f3, g2g3, h2h3, a2a4, b2b4, c2c4, d2d4, e2e4, f2f4, g2g4, h2h4, b1a3, b1c3, g1f3, g1h3)
        >>> 
        >>> board.gen_pseudo_legal_moves
        PseudoLegalMoveGenerator at 0x7e1c203695b0 (a2a3, b2b3, c2c3, d2d3, e2e3, f2f3, g2g3, h2h3, a2a4, b2b4, c2c4, d2d4, e2e4, f2f4, g2g4, h2h4, b1a3, b1c3, g1f3, g1h3)

* Show a simple ASCII board.

    .. code:: python

        >>> board = sisyphus.Board()
        >>> print(board)
        8 r n b q k b n r 
        7 p p p p p p p p 
        6 . . . . . . . . 
        5 . . . . . . . . 
        4 . . . . . . . . 
        3 . . . . . . . . 
        2 P P P P P P P P 
        1 R N B Q K B N R 
        a b c d e f g h

        >>> board.unicode() # For pretty-printing
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

* Make and unmake moves 

    .. code:: python 

        >>> move = sisyphus.Move.parse_uci(board, 'e2e3') # Or sisyphus.Move(sisyphus.E2, sisyphus.E3, sisyphus.PieceType(sisyphus.PAWN, sisyphus.WHITE))
        >>> move in board.gen_legal_moves
        True
        >>> board.push(move) # Make move

        >>> board.pop() # Unmake move
        Move(san=e2e3, from=12, to=20, piece=PieceType'P', flag=0)

* Detects checkmates, stalemates and draws by insufficient material.

    .. code:: python

        >>> board = sisyphus.Board("r1bqkb1r/pppp1Qpp/2n2n2/4p3/2B1P3/8/PPPP1PPP/RNB1K1NR b KQkq - 0 4")
        >>> board.is_stalemate()
        False 
        >>> board.drawn_by_insufficient_material()
        False 
        >>> board.is_checkmate()
        True 

* Detects checks and attacks.

    .. code:: python 

        >>> board.is_check()
        True 
        >>> board.is_square_attacked_by(sisyphus.WHITE, sisyphus.E8)
        True 

        >>> attackers = board.attackers(sisyphus.WHITE, sisyphus.F3)
        >>> attackers
        SquareSet(0x0000_0000_0000_4040)
        >>> sisyphus.G1 in attackers
        True
        >>> print(attackers)
        . . . . . . . .
        . . . . . . . .
        . . . . . . . .
        . . . . . . . .
        . . . . . . . .
        . . . . . . . .
        . . . . . . 1 .
        . . . . . . 1 .

* Parses and creates FENs.

    .. code:: python 

        >>> board.fen
        'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -'
        >>>
        >>> board = sisyphus.Board("rnbqkbnr/ppp1pppp/8/3p4/3P4/8/PPP1PPPP/RNBQKBNR b KQkq d3 0 1")
        >>> board.color_at(sisyphus.D1) # '0' for WHITE and '1' for BLACK
        0

* Performance test.

    .. code:: python 

        >>> board.clear()
        >>> board.perft_test(6)
        119060324 

* Running Tests

    .. code-block:: shell

        # Run all tests
        $ python3 tests.py

        # Run tests with verbose output
        $ python3 tests.py -v

Play against Sisyphus!
----------------------

You can play against Sisyphus using any chess GUI that supports the UCI protocol,
It has been primarily tested with `CuteChess <https://github.com/cutechess/cutechess>`_, It also has a `Lichess account <https://lichess.org/@/Sisyphus-engine>`_ where you can challenge it.

License
-------

Sisyphus is under the MIT License.
Check out ``LICENSE.txt`` for the full text.

Contributing
-----------

Contributions are welcome! Please feel free to submit a Pull Request.
