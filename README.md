# FoChess

Simple Chess engine written in C++ just for fun and experimenting.

At the moment the engine is partially UCI-compliant and can be tested on applications such as cutechess. At the moment the UCI engine class is very bad but it is not yet important. 
It might sometimes make an illegal move because of possible TTs collisions which will be fixed after improving the search to be even faster. 
There is no iterative deepening so considering a good computer I would recommend a depth between 6 and 8 plies for testing. 
I did not yet try to make some tests against other chess engines but I guess I might have the record for lowest ELO engine :D. Ok, not really but you get the point.

At the moment these are the features:
    - A very bad alpha pruning implementation 
    - A very bad search heuristic 
    - A kinda good movegen tested with perft
    - Transposition Tables
    - Zobrist hashing
