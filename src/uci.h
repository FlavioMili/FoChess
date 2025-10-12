// -----------------------------------------------------------------------------
//  FoChess
//  Copyright (c) 2025 Flavio Milinanni. All Rights Reserved.
//
//  Read the LICENSE file in the project root please.
// -----------------------------------------------------------------------------

#pragma once

#include <string>

#include "board.h"
#include "tt.h"

class UCIengine {
 public:
  UCIengine();
  void loop();

 private:
  void uci();
  void isready();
  void position(std::string& line);
  void go(std::string& line);
  void stop();
  void ucinewgame();
  void info(int depth, int score);

  Board board;
  TranspositionTable tt;
};

namespace UCI {

// main method to loop input output
void loop();

/**********************************************************************************
********************************** GUI TO ENGINE **********************************
**********************************************************************************/

/** tell engine to use the uci (universal chess interface),
  this will be send once as a first command after program boot
  to tell the engine to switch to uci mode.
  After receiving the uci command the engine must identify itself with the "id" command
  and sent the "option" commands to tell the GUI which engine settings the engine supports if any.
  After that the engine should sent "uciok" to acknowledge the uci mode.
  If no uciok is sent within a certain time period, the engine task will be killed by the GUI.
*/
void uci();

/*
  switch the debug mode of the engine on and off.
  In debug mode the engine should sent additional infos to the GUI, e.g. with the "info string"
  command, to help debugging, e.g. the commands that the engine has received etc. This mode should
  be switched off by default and this command can be sent any time, also when the engine is
  thinking.
*/
void debug();

/*
  this is used to synchronize the engine with the GUI. When the GUI has sent a command or
  multiple commands that can take some time to complete,
  this command can be used to wait for the engine to be ready again or
  to ping the engine to find out if it is still alive.
  E.g. this should be sent after setting the path to the tablebases as this can take some time.
  This command is also required once before the engine is asked to do any search
  to wait for the engine to finish initializing.
  This command must always be answered with "readyok" and can be sent also when the engine is
  calculating in which case the engine should also immediately answer with "readyok" without
  stopping the search.
*/
void isready();

// not implemented just keeping the place here to keep the protocol specification order.
// void setoption(std::string name);
// void register();

/*
   this is sent to the engine when the next search (started with "position" and "go") will be from
   a different game. This can be a new game the engine should play or a new game it should analyse
   but also the next position from a testsuite with positions only. If the GUI hasn't sent a
   "ucinewgame" before the first "position" command, the engine shouldn't expect any further
   ucinewgame commands as the GUI is probably not supporting the ucinewgame command. So the engine
   should not rely on this command even though all new GUIs should support it. As the engine's
   reaction to "ucinewgame" can take some time the GUI should always send "isready" after
   "ucinewgame" to wait for the engine to finish its operation.
*/
void ucinewgame();

/*
  set up the position described in fenstring on the internal board and
  play the moves on the internal chess board.
  if the game was played  from the start position the string "startpos" will be sent
  Note: no "new" command is needed. However, if this position is from a different game than
  the last position sent to the engine, the GUI should have sent a "ucinewgame" inbetween.
*/
void position(std::string line);

/*
  start calculating on the current position set up with the "position" command.
  There are a number of commands that can follow this command, all will be sent in the same string.
  If one command is not send its value should be interpreted as it would not influence the search.
  * searchmoves  ....
    restrict search to this moves only
    Example: After "position startpos" and "go infinite searchmoves e2e4 d2d4"
    the engine should only search the two moves e2e4 and d2d4 in the initial position.
  * ponder
    start searching in pondering mode.
    Do not exit the search in ponder mode, even if it's mate!
    This means that the last move sent in in the position string is the ponder move.
    The engine can do what it wants to do, but after a "ponderhit" command
    it should execute the suggested move to ponder on. This means that the ponder move sent by
    the GUI can be interpreted as a recommendation about which move to ponder. However, if the
    engine decides to ponder on a different move, it should not display any mainlines as they are
    likely to be misinterpreted by the GUI because the GUI expects the engine to ponder
     on the suggested move.
  * wtime
    white has x msec left on the clock
  * btime
    black has x msec left on the clock
  * winc
    white increment per move in mseconds if x > 0
  * binc
    black increment per move in mseconds if x > 0
  * movestogo
      there are x moves to the next time control,
    this will only be sent if x > 0,
    if you don't get this and get the wtime and btime it's sudden death
  * depth
    search x plies only.
  * nodes
     search x nodes only,
  * mate
    search for a mate in x moves
  * movetime
    search exactly x mseconds
  * infinite
    search until the "stop" command. Do not exit the search without being told so in this mode!
*/
void go();

/*
  stop calculating as soon as possible,
  don't forget the "bestmove" and possibly the "ponder" token when finishing the search
*/
void stop();

// void ponderhit();

// quit the program as soon as possible
void quit();

/**********************************************************************************
********************************** ENGINE TO GUI **********************************
**********************************************************************************/
/*
  * name
    this must be sent after receiving the "uci" command to identify the engine,
    e.g. "id name Shredder X.Y\n"
  * author
    this must be sent after receiving the "uci" command to identify the engine,
    e.g. "id author Stefan MK\n"
*/
void id();

/*
  Must be sent after the id and optional options to tell the GUI that the engine
  has sent all infos and is ready in uci mode.
*/
void uciok();

/*
  This must be sent when the engine has received an "isready" command and has
  processed all input and is ready to accept new commands now.
  It is usually sent after a command that can take some time to be able to wait for the engine,
  but it can be used anytime, even when the engine is searching,
  and must always be answered with "isready".
*/
void readyok();

/*
  the engine has stopped searching and found the move  best in this position.
  the engine can send the move it likes to ponder on. The engine must not start pondering
  automatically. this command must always be sent if the engine stops searching, also in pondering
  mode if there is a "stop" command, so for every "go" command a "bestmove" command is needed!
  Directly before that the engine should send a final "info" command with the final search
  information, the the GUI has the complete statistics about the last search.
*/
void bestmove();

// void copyprotection(); unused ?

// void registration(); not yet, TODO make it in the future

/*
  the engine wants to send infos to the GUI. This should be done whenever one of the info has
  changed. The engine can send only selected infos and multiple infos can be send with one info
  command, e.g. "info currmove e2e4 currmovenumber 1" or "info depth 12 nodes 123456 nps 100000".
  Also all infos belonging to the pv should be sent together
  e.g. "info depth 2 score cp 214 time 1242 nodes 2124 nps 34928 pv e2e4 e7e5 g1f3"
  I suggest to start sending "currmove", "currmovenumber", "currline" and "refutation" only after
  one second to avoid too much traffic. Additional info:
  * depth
    search depth in plies
  * seldepth
    selective search depth in plies,
    if the engine sends seldepth there must also a "depth" be present in the same string.
  * time
    the time searched in ms, this should be sent together with the pv.
  * nodes
    x nodes searched, the engine should send this info regularly
  * pv  ...
    the best line found
  * multipv
    this for the multi pv mode.
    for the best move/pv add "multipv 1" in the string when you send the pv.
    in k-best mode always send all k variants in k strings together.
  * score
    * cp
      the score from the engine's point of view in centipawns.
    * mate
      mate in y moves, not plies.
      If the engine is getting mated use negativ values for y.
    * lowerbound
        the score is just a lower bound.
    * upperbound
     the score is just an upper bound.
  * currmove
    currently searching this move
  * currmovenumber
    currently searching move number x, for the first move x should be 1 not 0.
  * hashfull
    the hash is x permill full, the engine should send this info regularly
  * nps
    x nodes per second searched, the engine should send this info regularly
  * tbhits
    x positions where found in the endgame table bases
  * cpuload
    the cpu usage of the engine is x permill.
  * string
    any string str which will be displayed be the engine,
    if there is a string command the rest of the line will be interpreted as .
  * refutation   ...
     move  is refuted by the line  ... , i can be any number >= 1.
     Example: after move d1h5 is searched, the engine can send
     "info refutation d1h5 g6h5"
     if g6h5 is the best answer after d1h5 or if g6h5 refutes the move d1h5.
     if there is norefutation for d1h5 found, the engine should just send
     "info refutation d1h5"
    The engine should only send this if the option "UCI_ShowRefutations" is set to true.
  * currline   ...
     this is the current line the engine is calculating.  is the number of the cpu if
     the engine is running on more than one cpu.  = 1,2,3....
     if the engine is just using one cpu,  can be omitted.
     If  is greater than 1, always send all k lines in k strings together.
    The engine should only send this if the option "UCI_ShowCurrLine" is set to true.
*/
void info();

/*
  This command tells the GUI which parameters can be changed in the engine.
  This should be sent once at engine startup after the "uci" and the "id" commands
  if any parameter can be changed in the engine.
  The GUI should parse this and build a dialog for the user to change the settings.
  Note that not every option needs to appear in this dialog as some options like
  "Ponder", "UCI_AnalyseMode", etc. are better handled elsewhere or are set automatically.
  If the user wants to change some settings, the GUI will send a "setoption" command to the engine.
  Note that the GUI need not send the setoption command when starting the engine for every option if
  it doesn't want to change the default value.
  For all allowed combinations see the example below,
  as some combinations of this tokens don't make sense.
  One string will be sent for each parameter.
*/
void option();

}  // namespace UCI
