#pragma once
#include "Types.hpp"
#include <cstdint>

using Bitboard = uint64_t;

inline constexpr void set_bit(Bitboard& board, Square sq) noexcept {
    board |= (1ULL << static_cast<uint8_t>(sq));
}

inline constexpr void clear_bit(Bitboard& board, Square sq) noexcept {
    board &= ~(1ULL << static_cast<uint8_t>(sq));
}

//passing a 64 bit integer by reference is slower than psasing it by value;
[[nodiscard]] inline constexpr bool get_bit(Bitboard board, Square sq) noexcept {
    return (board & (1ULL << static_cast<uint8_t>(sq))) != 0;
}