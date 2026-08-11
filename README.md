# FoChess

Simple Chess engine written in C++ just for fun and experimenting.

The engine is partially UCI-compliant and can be tested in applications such as
Cute Chess. Search uses iterative deepening and keeps the last fully completed
iteration when its time expires.

At the moment these are the features:
- Alpha-beta with principal-variation search and quiescence
- TT, MVV-LVA, killer, and history move ordering
- Move generation tested with perft
- Transposition tables with mate-distance normalization
- Zobrist hashing
