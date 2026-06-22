#pragma once
#include "Board.hpp"
#include "Types.hpp"

constexpr int PAWN_VAL   = 100;
constexpr int KNIGHT_VAL = 300;
constexpr int BISHOP_VAL = 300;
constexpr int ROOK_VAL   = 500;
constexpr int QUEEN_VAL  = 900;
constexpr int KING_VAL = 20000;

template<Color side>
[[nodiscard]] int evaluate(const BoardState& board) noexcept;

