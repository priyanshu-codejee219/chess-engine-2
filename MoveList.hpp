#pragma once
#include "Types.hpp"
#include <array>

class MoveList{
private: 
    //maximu possible moves ai any given configuration does not exceed 218, so we use 256
    std::array<Move, 256> moves{}; // this is a stack array.
    int count=0; //this is used to reset moves. we do not reset complete moves at any time.

public:
    inline constexpr void push(Move move) noexcept{
        moves[count++] = move;
    }

    [[nodiscard]] inline constexpr int size() const noexcept{
        return count;
    }


    // begin and end allows us to use standard for loops on this.
    [[nodiscard]] inline constexpr Move* begin() noexcept{
        return moves.data();
    }

    [[nodiscard]] inline constexpr Move* end() noexcept{
        return moves.data() + count;
    }
};