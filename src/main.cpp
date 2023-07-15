/*
  Stockfish, a UCI chess playing engine derived from Glaurung 2.1
  Copyright (C) 2004-2023 The Stockfish developers (see AUTHORS file)

  Stockfish is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Stockfish is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include "bitboard.h"
#include "position.h"
#include "psqt.h"
#include "search.h"
#include "syzygy/tbprobe.h"
#include "thread.h"
#include "tt.h"
#include "uci.h"

int sf_init();
#ifndef shared_lib
void input_reader();
#endif

using namespace Stockfish;

int sf_init() {
  sync_cout << engine_info() << sync_endl;
  UCI::init(Options);
  Tune::init();
  PSQT::init();
  Bitboards::init();
  Position::init();
  Threads.set(size_t(Options["Threads"]));
  Search::clear(); // After threads are up
  Eval::NNUE::init();
  UCI::init_pos();
  return 0;
}

#ifndef shared_lib
/// input_reader() waits for a command from stdin and invokes UCI::execute()
/// Also intercepts EOF from stdin to ensure gracefully exiting if the
/// GUI dies unexpectedly.
void input_reader() {
    std::string cmd;
    while (getline(std::cin, cmd)) {
#ifndef NDEBUG
        if (cmd.substr(0, 1) == "#") {
            continue;
        }
        if (cmd.substr(0, 5) == "wait ") {
            int time = std::stoi(cmd.substr(5));
#if defined(__MINGW32__) || defined(__MINGW64__)
            Sleep(time);
#else
            sleep(time);
#endif
            continue;
        }
#endif
        UCI::execute(cmd);
        if (cmd == "quit")
            break;
    }
}

/// When SF is called with some command line arguments, e.g. to
/// run 'bench', once the command is executed the program stops.
int main(int argc, char* argv[]) {

  CommandLine::init(argc, argv);
  int res = sf_init();

  if (argc > 1) {
    std::string cmd;
    for (int i = 1; i < argc; ++i)
      cmd += std::string(argv[i]) + " ";
    UCI::execute(cmd);
#if defined(__MINGW32__) || defined(__MINGW64__)
    Sleep(1);
#else
    sleep(1);
#endif
    UCI::execute("quit");
  } else {
    std::thread input_reader_thread(input_reader);
    input_reader_thread.join();
  }
  return res;
}
#endif
