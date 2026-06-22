#pragma once
#include "Types.hpp"
#include "Bitboard.hpp"
#include <cstdint>
#include <array>
#include <cmath>

// 15 means 1111 in binary (no rights lost).
// E1 (index 4) = 12 (strips both White rights)
// A1 (index 0) = 13 (strips White Queenside)
// H1 (index 7) = 14 (strips White Kingside)
constexpr uint8_t CASTLING_RIGHTS_MASK[64] = {
    13, 15, 15, 15, 12, 15, 15, 14,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    7,  15, 15, 15, 3,  15, 15, 11
};

//alignas ensures that each instance of this struct gets allocated at beginning of a cache line (L1 cache)
//if a object shared between two chunks of 64 bytes, 2 slow calls will be called. 
//if we do multithreading and teo objects get allocated into the same chunk, then thread 1 when explores the first object, 
//it deletes everything present in the cache line, so 2nd thread has to go all the way to the main memory to retrienve the second object
struct alignas(64) BoardState{
    std::array<Bitboard, 12> pieces{};
    std::array<Bitboard, 3> occupancies{};
    CastleRights castle_rights{};
    Square en_passant_square = Square::None;

    [[nodiscard]] constexpr Bitboard get_occupied() const noexcept {
        return occupancies[static_cast<uint8_t>(Color::Both)];
    }

    [[nodiscard]] constexpr BoardState make_move(Move move) const noexcept {
        BoardState next_state = *this;

        Square from = move.get_start();
        Square to = move.get_end();

        //get piece which moves.
        int moving_piece=-1;
        for(int i=0; i<12; i++){
            if(get_bit(next_state.pieces[i], from)){
                moving_piece = i;
                break;
            }
        }

        //get piece at cell where to move
        int captured_piece = -1;
        for(int i=0; i<12; i++){
            if(get_bit(next_state.pieces[i], to)){
                captured_piece = i;
                break;
            }
        }

        //move and clear in next stae
        clear_bit(next_state.pieces[moving_piece], from);
        set_bit(next_state.pieces[moving_piece], to);

        if (captured_piece != -1) {
            clear_bit(next_state.pieces[captured_piece], to);
        }

        if (moving_piece == static_cast<uint8_t>(PieceType::WhiteKing) && from == Square::E1) {
            if (to == Square::G1) {
                clear_bit(next_state.pieces[static_cast<uint8_t>(PieceType::WhiteRook)], Square::H1);
                set_bit(next_state.pieces[static_cast<uint8_t>(PieceType::WhiteRook)], Square::F1);
            } else if (to == Square::C1) {
                clear_bit(next_state.pieces[static_cast<uint8_t>(PieceType::WhiteRook)], Square::A1);
                set_bit(next_state.pieces[static_cast<uint8_t>(PieceType::WhiteRook)], Square::D1);
            }
        } else if (moving_piece == static_cast<uint8_t>(PieceType::BlackKing) && from == Square::E8) {
            if (to == Square::G8) {
                clear_bit(next_state.pieces[static_cast<uint8_t>(PieceType::BlackRook)], Square::H8);
                set_bit(next_state.pieces[static_cast<uint8_t>(PieceType::BlackRook)], Square::F8);
            } else if (to == Square::C8) {
                clear_bit(next_state.pieces[static_cast<uint8_t>(PieceType::BlackRook)], Square::A8);
                set_bit(next_state.pieces[static_cast<uint8_t>(PieceType::BlackRook)], Square::D8);
            }
        }

        // EN PASSANT LOGIC
        next_state.en_passant_square = Square::None;

        if (moving_piece == static_cast<uint8_t>(PieceType::WhitePawn) || 
            moving_piece == static_cast<uint8_t>(PieceType::BlackPawn)) {
            
            if (std::abs(static_cast<int>(to) - static_cast<int>(from)) == 16) {
                next_state.en_passant_square = static_cast<Square>((static_cast<int>(from) + static_cast<int>(to)) / 2);
            }
            else if (captured_piece == -1 && (std::abs(static_cast<int>(to) - static_cast<int>(from)) == 7 || 
                                              std::abs(static_cast<int>(to) - static_cast<int>(from)) == 9)) {
                int enemy_pawn_sq = (moving_piece == static_cast<uint8_t>(PieceType::WhitePawn)) ? static_cast<int>(to) - 8 : static_cast<int>(to) + 8;
                int enemy_pawn_type = (moving_piece == static_cast<uint8_t>(PieceType::WhitePawn)) ? static_cast<uint8_t>(PieceType::BlackPawn) : static_cast<uint8_t>(PieceType::WhitePawn);
                clear_bit(next_state.pieces[enemy_pawn_type], static_cast<Square>(enemy_pawn_sq));
            }
        }
        
        // PROMOTION LOGIC
        uint16_t flag = move.get_flag();
        if (flag != FLAG_NONE) {
            clear_bit(next_state.pieces[moving_piece], to);
            int promo_offset = (moving_piece == static_cast<uint8_t>(PieceType::WhitePawn)) ? 0 : 6;
            if (flag == FLAG_PROMO_KNIGHT) set_bit(next_state.pieces[static_cast<uint8_t>(PieceType::WhiteKnight) + promo_offset], to);
            else if (flag == FLAG_PROMO_BISHOP) set_bit(next_state.pieces[static_cast<uint8_t>(PieceType::WhiteBishop) + promo_offset], to);
            else if (flag == FLAG_PROMO_ROOK) set_bit(next_state.pieces[static_cast<uint8_t>(PieceType::WhiteRook) + promo_offset], to);
            else if (flag == FLAG_PROMO_QUEEN) set_bit(next_state.pieces[static_cast<uint8_t>(PieceType::WhiteQueen) + promo_offset], to);
        }

        next_state.occupancies[static_cast<uint8_t>(Color::White)] = 0;
        next_state.occupancies[static_cast<uint8_t>(Color::Black)] = 0;

        for(int i = 0; i < 6; i++) {
            next_state.occupancies[static_cast<uint8_t>(Color::White)] |= next_state.pieces[i];
        }
        for(int i = 6; i < 12; i++) {
            next_state.occupancies[static_cast<uint8_t>(Color::Black)] |= next_state.pieces[i];
        }
        
        next_state.occupancies[static_cast<uint8_t>(Color::Both)] = 
            next_state.occupancies[static_cast<uint8_t>(Color::White)] | 
            next_state.occupancies[static_cast<uint8_t>(Color::Black)];

        // 6. Update Castling Rights instantly
        // We apply the mask to BOTH the 'from' and 'to' squares. 
        // If a Rook is captured on its starting square, the enemy loses the right to castle with it!
        next_state.castle_rights.rights &= CASTLING_RIGHTS_MASK[static_cast<uint8_t>(from)];
        next_state.castle_rights.rights &= CASTLING_RIGHTS_MASK[static_cast<uint8_t>(to)];

        return next_state;
    }
};