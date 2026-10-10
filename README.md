# FoChess

Simple chess engine written in C++ just for fun and experimenting.

The engine is partially UCI-compliant and can be tested in applications such as
Cute Chess. Search uses iterative deepening and keeps the last fully completed
iteration when its time expires.

## Features

- Alpha-beta with principal-variation search and quiescence
- TT, MVV-LVA, killer, and history move ordering
- Move generation tested with perft
- Transposition tables with mate-distance normalization
- Zobrist hashing

## Requirements

- A C++23 compiler (`clang++` is preferred automatically when available; `g++` works too)
- `make`
- GoogleTest development package discoverable through `pkg-config` for running tests

On Debian/Ubuntu-like systems:

```sh
sudo apt install build-essential clang lld pkg-config libgtest-dev
```

## Build

Build both debug tools/tests and release tools:

```sh
make
```

Or build only one configuration:

```sh
make debug
make release
```

Release binaries are written to `build/release/`; debug binaries and the test
runner are written to `build/debug/`.

You can override the compiler if needed:

```sh
make CXX=g++ release
```

Clean generated files with:

```sh
make clean
```

## Run the engine

Build the release UCI executable:

```sh
make release
./build/release/uci_engine
```

The engine reads UCI commands from standard input, so it can also be connected to
a UCI GUI such as Cute Chess. For a quick smoke test:

```sh
printf "uci\nisready\nposition startpos\ngo depth 4\nquit\n" | ./build/release/uci_engine
```

## Benchmark

Run the regular search benchmark over `resources/bench_fen_list.txt`. The
optional argument is the search depth, defaulting to `5`; depth `9` takes about
a second and is a steadier speed measurement:

```sh
make release
./build/release/bench
./build/release/bench 9
```

Build profiling-oriented binaries with symbols and release optimizations:

```sh
make perf
./build/perf/bench
```

Run the branching/diagnostics benchmark. The optional argument is the maximum
search depth, defaulting to `8`:

```sh
make branching
./build/perf/branching_bench 8
```

## Perft

`perft` is useful for validating move generation. The first argument is depth;
if no FEN is supplied, the standard starting position is used. It runs on one
thread unless `-t` asks for more.

```sh
make release
./build/release/perft 5
./build/release/perft 4 "r3k2r/p1ppqpb1/bn2pnp1/2pP4/1p2P3/2N2N2/PPPB1PPP/R2QKB1R w KQkq -"
./build/release/perft 7 -t 16
```

The program prints a divide by root move, the total node count, and the speed.

Both builds use the engine's `MoveGen::generate_all` and `Board::makeMove`,
sharing the simple perft implementation in `tools/perft.h` with the test suite.
Bulk counting is the default: the final ply counts legal moves directly. The
release build can distribute root moves across threads; the debug runner uses
non-bulk counting on one thread to report captures and checks. Perft does not
use a transposition table or cache subtree node counts.

## Test

Run the unified GoogleTest test binary:

```sh
make test
```

This builds and runs `build/debug/functionality_test`, covering FEN handling,
move generation/perft, moving, search regressions, transposition-table behavior,
and UCI behavior.
