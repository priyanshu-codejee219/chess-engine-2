#pragma once
#include "Bitboard.hpp"
#include "Board.hpp"   
#include "MoveList.hpp"
#include <cstdint>
#include <array>

constexpr Bitboard NOT_A_FILE=0xfefefefefefefefeULL;
constexpr Bitboard NOT_AB_FILE=0xfcfcfcfcfcfcfcfcULL;
constexpr Bitboard NOT_H_FILE=0x7f7f7f7f7f7f7f7fULL;
constexpr Bitboard NOT_GH_FILE=0x3f3f3f3f3f3f3f3fULL;


// KNIGHTS

[[nodiscard]] constexpr Bitboard mask_knight_attacks(Square square) noexcept{
    Bitboard bitboard=0;
    set_bit(bitboard,square);
    Bitboard attacks=0;
    attacks|=(bitboard<<17)&NOT_A_FILE;
    attacks|=(bitboard<<15)&NOT_H_FILE;
    attacks|=(bitboard<<10)&NOT_AB_FILE;
    attacks|=(bitboard<<6)&NOT_GH_FILE;
    attacks|=(bitboard>>15)&NOT_A_FILE;
    attacks|=(bitboard>>17)&NOT_H_FILE;
    attacks|=(bitboard>>6)&NOT_AB_FILE;
    attacks|=(bitboard>>10)&NOT_GH_FILE;
    return attacks;
}

[[nodiscard]] constexpr std::array<Bitboard,64> init_knight_attacks() noexcept{
    std::array<Bitboard,64> attacks{};
    for(int i=0;i<=63;i++){
        attacks[i]=mask_knight_attacks(static_cast<Square>(i));
    }
    return attacks;
}


// KINGS

[[nodiscard]] constexpr Bitboard mask_king_attacks(Square square) noexcept{
    Bitboard bitboard=0;
    set_bit(bitboard,square);
    Bitboard attacks=0;
    attacks|=(bitboard<<8);
    attacks|=(bitboard>>8);
    attacks|=(bitboard<<1)&NOT_A_FILE;
    attacks|=(bitboard>>1)&NOT_H_FILE;
    attacks|=(bitboard<<9)&NOT_A_FILE;
    attacks|=(bitboard<<7)&NOT_H_FILE;
    attacks|=(bitboard>>7)&NOT_A_FILE;
    attacks|=(bitboard>>9)&NOT_H_FILE;
    return attacks;
}

[[nodiscard]] constexpr std::array<Bitboard,64> init_king_attacks() noexcept{
    std::array<Bitboard,64> attacks{};
    for(int i=0;i<=63;i++){
        attacks[i]=mask_king_attacks(static_cast<Square>(i));
    }
    return attacks;
}


// PAWNS (DIAGONAL ATTACKS ONLY)

[[nodiscard]] constexpr Bitboard mask_white_pawn_attacks(Square square) noexcept{
    Bitboard bitboard=0;
    set_bit(bitboard,square);
    Bitboard attacks=0;
    attacks|=(bitboard<<9)&NOT_A_FILE;
    attacks|=(bitboard<<7)&NOT_H_FILE;
    return attacks;
}

[[nodiscard]] constexpr std::array<Bitboard,64> init_white_pawn_attacks() noexcept{
    std::array<Bitboard,64> attacks{};
    for(int i=0;i<=63;i++){
        attacks[i]=mask_white_pawn_attacks(static_cast<Square>(i));
    }
    return attacks;
}

[[nodiscard]] constexpr Bitboard mask_black_pawn_attacks(Square square) noexcept{
    Bitboard bitboard=0;
    set_bit(bitboard,square);
    Bitboard attacks=0;
    attacks|=(bitboard>>7)&NOT_A_FILE;
    attacks|=(bitboard>>9)&NOT_H_FILE;
    return attacks;
}

[[nodiscard]] constexpr std::array<Bitboard,64> init_black_pawn_attacks() noexcept{
    std::array<Bitboard,64> attacks{};
    for(int i=0;i<=63;i++){
        attacks[i]=mask_black_pawn_attacks(static_cast<Square>(i));
    }
    return attacks;
}


// BISHOPS (EMPTY BOARD RAYS)

[[nodiscard]] constexpr Bitboard mask_bishop_attacks(Square square) noexcept{
    Bitboard attacks=0;
    int r=static_cast<int>(square)/8;
    int f=static_cast<int>(square)%8;
    
    for(int i=r+1,j=f+1;i<=7&&j<=7;i++,j++) attacks|=(1ULL<<(i*8+j));
    for(int i=r+1,j=f-1;i<=7&&j>=0;i++,j--) attacks|=(1ULL<<(i*8+j));
    for(int i=r-1,j=f+1;i>=0&&j<=7;i--,j++) attacks|=(1ULL<<(i*8+j));
    for(int i=r-1,j=f-1;i>=0&&j>=0;i--,j--) attacks|=(1ULL<<(i*8+j));
    return attacks;
}

[[nodiscard]] constexpr std::array<Bitboard,64> init_bishop_attacks() noexcept{
    std::array<Bitboard,64> attacks{};
    for(int i=0;i<=63;i++){
        attacks[i]=mask_bishop_attacks(static_cast<Square>(i));
    }
    return attacks;
}


// ROOKS (EMPTY BOARD RAYS)

[[nodiscard]] constexpr Bitboard mask_rook_attacks(Square square) noexcept{
    Bitboard attacks=0;
    int r=static_cast<int>(square)/8;
    int f=static_cast<int>(square)%8;
    
    for(int i=r+1;i<=7;i++) attacks|=(1ULL<<(i*8+f));
    for(int i=r-1;i>=0;i--) attacks|=(1ULL<<(i*8+f));
    for(int i=f+1;i<=7;i++) attacks|=(1ULL<<(r*8+i));
    for(int i=f-1;i>=0;i--) attacks|=(1ULL<<(r*8+i));
    return attacks;
}

[[nodiscard]] constexpr std::array<Bitboard,64> init_rook_attacks() noexcept{
    std::array<Bitboard,64> attacks{};
    for(int i=0;i<=63;i++){
        attacks[i]=mask_rook_attacks(static_cast<Square>(i));
    }
    return attacks;
}


// QUEENS (EMPTY BOARD RAYS)

[[nodiscard]] constexpr Bitboard mask_queen_attacks(Square square) noexcept{
    return mask_rook_attacks(square)|mask_bishop_attacks(square);
}

[[nodiscard]] constexpr std::array<Bitboard,64> init_queen_attacks() noexcept{
    std::array<Bitboard,64> attacks{};
    for(int i=0;i<=63;i++){
        attacks[i]=mask_queen_attacks(static_cast<Square>(i));
    }
    return attacks;
}


// GLOBAL COMPILE-TIME LOOKUP TABLES

inline constexpr std::array<Bitboard,64> knightAttacks=init_knight_attacks();
inline constexpr std::array<Bitboard,64> kingAttacks=init_king_attacks();
inline constexpr std::array<Bitboard,64> whitePawnAttacks=init_white_pawn_attacks();
inline constexpr std::array<Bitboard,64> blackPawnAttacks=init_black_pawn_attacks();
inline constexpr std::array<Bitboard,64> bishopAttacks=init_bishop_attacks();
inline constexpr std::array<Bitboard,64> rookAttacks=init_rook_attacks();
inline constexpr std::array<Bitboard,64> queenAttacks=init_queen_attacks();

// Forward declaration of our move generator so other files can see it
template<Color side>
[[nodiscard]] MoveList generate_moves(const BoardState& board) noexcept;


template<Color attacker_side>
[[nodiscard]] bool is_square_attacked(Square sq, const BoardState& board) noexcept;