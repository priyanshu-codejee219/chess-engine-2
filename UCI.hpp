#pragma once
#include "Board.hpp"
#include "Types.hpp"
#include <string>

// build BoardState using fen string
BoardState parse_fen(const std::string& fen);

// The main loop that listens to the Chess GUI
void uci_loop();