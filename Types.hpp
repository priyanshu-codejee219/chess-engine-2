#pragma once

//#include<iostream> never include iostream in header files. injects unnecessary lines of code.
#include<cstdint>

enum class Square : uint8_t {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8,
    None = 64
};

enum class Color : uint8_t{
    White=0,
    Black=1,
    Both=2
};

enum class PieceType:uint8_t{
    WhitePawn,WhiteKnight,WhiteBishop,WhiteRook,WhiteQueen,WhiteKing,
    BlackPawn,BlackKnight,BlackBishop,BlackRook,BlackQueen,BlackKing,
    None
};

// 4-bit Move Flags for Promotions
constexpr uint16_t FLAG_NONE = 0;
constexpr uint16_t FLAG_PROMO_KNIGHT = 1;
constexpr uint16_t FLAG_PROMO_BISHOP = 2;
constexpr uint16_t FLAG_PROMO_ROOK = 3;
constexpr uint16_t FLAG_PROMO_QUEEN = 4;

struct Move{
    uint16_t data=0; // 6 bits for start, 6 bits for end, 4 bits flag for special moves. 

    //using constexpr allows compile time evaluation whenever possible.
    [[nodiscard]] constexpr Square get_start() const noexcept{
        return static_cast<Square>(data & 0x3F); //63 -> masks 1st 6 bits
    }

    [[nodiscard]] constexpr Square get_end() const noexcept{
        return static_cast<Square>((data>>6) & 0x3F); //makss next 6 bits.
    }
    
    [[nodiscard]] constexpr uint16_t get_flag() const noexcept { 
        return (data >> 12) & 0x0F; 
    }

    // Use bitmasking
    // Square start;
    // Square end;
    // PieceType pieceType;
    // bool promotion;
    // bool castling;
    // bool enpassant;
};

// nodiscard -> It makes no sense if these functions are called but the return values is not stored anywhere. To ensure that it is not wasted, we write this.
// noexcept -> We are telling compiler that this function will never throw. The compiler now does not have to deal with the exception handling for this function, generating straightforward fast machine code. 
struct CastleRights{
    static constexpr uint8_t WK = 1;
    static constexpr uint8_t WQ = 2;
    static constexpr uint8_t BK = 4;
    static constexpr uint8_t BQ = 8;
    
    std::uint8_t rights=0;
    
    [[nodiscard]] constexpr bool has_wk() const noexcept { return rights & WK; }
    [[nodiscard]] constexpr bool has_wq() const noexcept { return rights & WQ; }
    [[nodiscard]] constexpr bool has_bk() const noexcept { return rights & BK; }
    [[nodiscard]] constexpr bool has_bq() const noexcept { return rights & BQ; }

    constexpr void remove_wk() noexcept { rights &= ~WK; }
    constexpr void remove_wq() noexcept { rights &= ~WQ; }
    constexpr void remove_bk() noexcept { rights &= ~BK; }
    constexpr void remove_bq() noexcept { rights &= ~BQ; }
};