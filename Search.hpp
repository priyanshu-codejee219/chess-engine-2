#pragma once
#include "Board.hpp"
#include "Types.hpp"

const int INF = 30000;
const int MATE_VALUE = 29000;

template<Color side>
[[nodiscard]] int quiescence_search(const BoardState& board, int alpha, int beta, int ply) noexcept;

template<Color side>
[[nodiscard]] int search(const BoardState& bst, int depth, int alpha, int beta, int ply) noexcept;

template<Color side>
[[nodiscard]] Move get_best_move(const BoardState& bst, int depth) noexcept;

