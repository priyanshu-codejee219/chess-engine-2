#include "MoveGen.hpp"
#include "MoveList.hpp"
#include "Board.hpp"
#include <cstdint>
#include <array>

//Extracts the index of the first '1' and instantly deletes it.
inline int pop_lsb(Bitboard& b) noexcept {
    int sq = __builtin_ctzll(b);
    b &= b - 1; 
    return sq;
}

// We use a template so the compiler generates two blazing-fast branchless versions of this function: one for White, one for Black.
template<Color side>
[[nodiscard]] MoveList generate_moves(const BoardState& board) noexcept {
    MoveList move_list;
    
    // who is moving?
    constexpr Color us = side;
    constexpr Color them = static_cast<Color>(static_cast<uint8_t>(side) ^ 1);
    
    // our pieces
    Bitboard friendly_occupancy = board.occupancies[static_cast<uint8_t>(us)];
    Bitboard enemy_occupancy = board.occupancies[static_cast<uint8_t>(them)];
    Bitboard total_occupancy = board.get_occupied();

    //knight
    constexpr PieceType knight_type = (us == Color::White) ? PieceType::WhiteKnight : PieceType::BlackKnight;
    Bitboard knights = board.pieces[static_cast<uint8_t>(knight_type)];

    while (knights) {
        int from_sq = pop_lsb(knights);
        
        // Look up attacks, mask out squares where our own pieces are sitting
        Bitboard attacks = knightAttacks[from_sq] & ~friendly_occupancy;
        
        while (attacks) {
            int to_sq = pop_lsb(attacks);
            
            // Push the move directly to our zero-allocation stack array
            Move m;
            m.data = static_cast<uint16_t>(from_sq) | (static_cast<uint16_t>(to_sq) << 6);
            move_list.push(m);
        }
    }

    //king
    constexpr PieceType king_type = (us == Color::White) ? PieceType::WhiteKing : PieceType::BlackKing;
    Bitboard kings = board.pieces[static_cast<uint8_t>(king_type)];

    while (kings) {
        int from_sq = pop_lsb(kings);
        Bitboard attacks = kingAttacks[from_sq] & ~friendly_occupancy;
        
        while (attacks) {
            int to_sq = pop_lsb(attacks);
            
            Move m;
            m.data = static_cast<uint16_t>(from_sq) | (static_cast<uint16_t>(to_sq) << 6);
            move_list.push(m);
        }
    }

    // 2.5 CASTLING
    if constexpr (us == Color::White) {
    if (board.castle_rights.has_wk()) {
        if (!(total_occupancy & 0x60ULL)
            && !is_square_attacked<them>(Square::E1, board)
            && !is_square_attacked<them>(Square::F1, board)
            && !is_square_attacked<them>(Square::G1, board)) {
            Move m;
            m.data = static_cast<uint16_t>(Square::E1) | (static_cast<uint16_t>(Square::G1) << 6);
            move_list.push(m);
        }
    }
    if (board.castle_rights.has_wq()) {
        if (!(total_occupancy & 0x0EULL)
            && !is_square_attacked<them>(Square::E1, board)
            && !is_square_attacked<them>(Square::D1, board)
            && !is_square_attacked<them>(Square::C1, board)) {
            Move m;
            m.data = static_cast<uint16_t>(Square::E1) | (static_cast<uint16_t>(Square::C1) << 6);
            move_list.push(m);
        }
    }
} else {
    if (board.castle_rights.has_bk()) {
        if (!(total_occupancy & 0x6000000000000000ULL)
            && !is_square_attacked<them>(Square::E8, board)
            && !is_square_attacked<them>(Square::F8, board)
            && !is_square_attacked<them>(Square::G8, board)) {
            Move m;
            m.data = static_cast<uint16_t>(Square::E8) | (static_cast<uint16_t>(Square::G8) << 6);
            move_list.push(m);
        }
    }
    if (board.castle_rights.has_bq()) {
        if (!(total_occupancy & 0x0E00000000000000ULL)
            && !is_square_attacked<them>(Square::E8, board)
            && !is_square_attacked<them>(Square::D8, board)
            && !is_square_attacked<them>(Square::C8, board)) {
            Move m;
            m.data = static_cast<uint16_t>(Square::E8) | (static_cast<uint16_t>(Square::C8) << 6);
            move_list.push(m);
        }
    }
}

    //Pawn logic
    constexpr PieceType pawn_type = (us == Color::White) ? PieceType::WhitePawn : PieceType::BlackPawn;
    Bitboard pawns = board.pieces[static_cast<uint8_t>(pawn_type)];

    auto add_pawn_move = [&](int from, int to) {
        if (to >= 56 || to <= 7) { 
            Move m1, m2, m3, m4;
            m1.data = static_cast<uint16_t>(from) | (static_cast<uint16_t>(to) << 6) | (FLAG_PROMO_QUEEN << 12);
            m2.data = static_cast<uint16_t>(from) | (static_cast<uint16_t>(to) << 6) | (FLAG_PROMO_ROOK << 12);
            m3.data = static_cast<uint16_t>(from) | (static_cast<uint16_t>(to) << 6) | (FLAG_PROMO_BISHOP << 12);
            m4.data = static_cast<uint16_t>(from) | (static_cast<uint16_t>(to) << 6) | (FLAG_PROMO_KNIGHT << 12);
            move_list.push(m1); move_list.push(m2); move_list.push(m3); move_list.push(m4);
        } else {
            Move m;
            m.data = static_cast<uint16_t>(from) | (static_cast<uint16_t>(to) << 6);
            move_list.push(m);
        }
    };

    while (pawns) {
        int from_sq = pop_lsb(pawns);
        
        // --- A. PUSHES ---
        // Calculate the square directly in front of the pawn
        int single_push_sq = (us == Color::White) ? (from_sq + 8) : (from_sq - 8);
        
        // A push is only valid if the target square is completely empty
        if (!get_bit(total_occupancy, static_cast<Square>(single_push_sq))) {
            add_pawn_move(from_sq, single_push_sq);
            
            // Check Double Push ONLY if the single push was valid
            // White pawns start on Rank 2 (Indices 8-15), Black on Rank 7 (Indices 48-55)
            bool is_start_rank = (us == Color::White && (from_sq >= 8 && from_sq <= 15)) || 
                                 (us == Color::Black && (from_sq >= 48 && from_sq <= 55));
                                 
            if (is_start_rank) {
                int double_push_sq = (us == Color::White) ? (from_sq + 16) : (from_sq - 16);
                if (!get_bit(total_occupancy, static_cast<Square>(double_push_sq))) {
                    Move m2;
                    m2.data = static_cast<uint16_t>(from_sq) | (static_cast<uint16_t>(double_push_sq) << 6);
                    move_list.push(m2);
                }
            }
        } 
    
        // --- B. CAPTURES ---
        // Look up diagonal attacks, mask strictly against ENEMY pieces
        Bitboard attacks = (us == Color::White) ? whitePawnAttacks[from_sq] : blackPawnAttacks[from_sq];
        attacks &= enemy_occupancy;
        
        while (attacks) {
            int to_sq = pop_lsb(attacks);
            add_pawn_move(from_sq, to_sq);
        }

        // --- C. EN PASSANT CAPTURES ---
        if (board.en_passant_square != Square::None) {
            Bitboard ep_mask = 0;
            set_bit(ep_mask, board.en_passant_square);
            
            Bitboard ep_attacks = (us == Color::White) ? whitePawnAttacks[from_sq] : blackPawnAttacks[from_sq];
            if (ep_attacks & ep_mask) {
                Move m_ep;
                m_ep.data = static_cast<uint16_t>(from_sq) | (static_cast<uint16_t>(board.en_passant_square) << 6);
                move_list.push(m_ep);
            }
        }
    }

    // 4. SLIDING PIECES (Bishops, Rooks, Queens)
    // Note: We are using ray-casting loops here to get the engine running. 
    // This will be upgraded to O(1) Magic Bitboards in the optimization phase.

    // Offsets for standard 1D array board traversal
    constexpr int bishop_offsets[4] = {9, 7, -9, -7};
    constexpr int rook_offsets[4]   = {8, -8, 1, -1};

    auto generate_slider_moves = [&](PieceType type, const int* offsets, int num_offsets) {
        Bitboard sliders = board.pieces[static_cast<uint8_t>(type)];
        while (sliders) {
            int from_sq = pop_lsb(sliders);
            
            for (int i = 0; i < num_offsets; ++i) {
                int to_sq = from_sq;
                while (true) {
                    // Check edge wrapping
                    int offset = offsets[i];
                    if (offset == 1 && (to_sq % 8 == 7)) break;   // Hit Right Edge
                    if (offset == -1 && (to_sq % 8 == 0)) break;  // Hit Left Edge
                    if ((offset == 9 || offset == -7) && (to_sq % 8 == 7)) break; // Right Diagonal Edge
                    if ((offset == 7 || offset == -9) && (to_sq % 8 == 0)) break; // Left Diagonal Edge
                    
                    to_sq += offset;
                    
                    // Check top/bottom board limits
                    if (to_sq < 0 || to_sq > 63) break;

                    // Hit a friendly piece? Stop the ray instantly.
                    if (get_bit(friendly_occupancy, static_cast<Square>(to_sq))) break;

                    // The square is valid (either empty or an enemy)
                    Move m;
                    m.data = static_cast<uint16_t>(from_sq) | (static_cast<uint16_t>(to_sq) << 6);
                    move_list.push(m);

                    // Hit an enemy piece? We capture it, but the ray must stop.
                    if (get_bit(enemy_occupancy, static_cast<Square>(to_sq))) break;
                }
            }
        }
    };

    // Generate White or Black sliders based on the 'us' template parameter
    generate_slider_moves((us == Color::White) ? PieceType::WhiteBishop : PieceType::BlackBishop, bishop_offsets, 4);
    generate_slider_moves((us == Color::White) ? PieceType::WhiteRook : PieceType::BlackRook, rook_offsets, 4);
    
    // The Queen is just a Bishop and a Rook combined!
    generate_slider_moves((us == Color::White) ? PieceType::WhiteQueen : PieceType::BlackQueen, bishop_offsets, 4);
    generate_slider_moves((us == Color::White) ? PieceType::WhiteQueen : PieceType::BlackQueen, rook_offsets, 4);


    return move_list;
}

template<Color attacker_side>
[[nodiscard]] bool is_square_attacked(Square sq, const BoardState& board) noexcept {
    // 1. Leapers (Reverse Perspective Lookup)
    constexpr PieceType knight = (attacker_side == Color::White) ? PieceType::WhiteKnight : PieceType::BlackKnight;
    if (knightAttacks[static_cast<uint8_t>(sq)] & board.pieces[static_cast<uint8_t>(knight)]) return true;
    
    constexpr PieceType king = (attacker_side == Color::White) ? PieceType::WhiteKing : PieceType::BlackKing;
    if (kingAttacks[static_cast<uint8_t>(sq)] & board.pieces[static_cast<uint8_t>(king)]) return true;
    
    // Pawns: If Black is attacking, pretend a WHITE pawn is on the square and see if it hits a Black pawn.
    constexpr PieceType pawn = (attacker_side == Color::White) ? PieceType::WhitePawn : PieceType::BlackPawn;
    Bitboard pawn_mask = (attacker_side == Color::White) ? blackPawnAttacks[static_cast<uint8_t>(sq)] : whitePawnAttacks[static_cast<uint8_t>(sq)];
    if (pawn_mask & board.pieces[static_cast<uint8_t>(pawn)]) return true;

    // 2. Sliders (Ray-Casting)
    constexpr PieceType bishop = (attacker_side == Color::White) ? PieceType::WhiteBishop : PieceType::BlackBishop;
    constexpr PieceType rook   = (attacker_side == Color::White) ? PieceType::WhiteRook   : PieceType::BlackRook;
    constexpr PieceType queen  = (attacker_side == Color::White) ? PieceType::WhiteQueen  : PieceType::BlackQueen;
    
    Bitboard total_occ = board.get_occupied();

    constexpr int bishop_offsets[4] = {9, 7, -9, -7};
    for (int offset : bishop_offsets) {
        int to_sq = static_cast<int>(sq);
        while (true) {
            if (offset == 1 && (to_sq % 8 == 7)) break;
            if (offset == -1 && (to_sq % 8 == 0)) break;
            if ((offset == 9 || offset == -7) && (to_sq % 8 == 7)) break;
            if ((offset == 7 || offset == -9) && (to_sq % 8 == 0)) break;
            to_sq += offset;
            if (to_sq < 0 || to_sq > 63) break;
            
            Square t = static_cast<Square>(to_sq);
            if (get_bit(board.pieces[static_cast<uint8_t>(bishop)], t) || get_bit(board.pieces[static_cast<uint8_t>(queen)], t)) return true;
            if (get_bit(total_occ, t)) break; 
        }
    }

    constexpr int rook_offsets[4] = {8, -8, 1, -1};
    for (int offset : rook_offsets) {
        int to_sq = static_cast<int>(sq);
        while (true) {
            if (offset == 1 && (to_sq % 8 == 7)) break;
            if (offset == -1 && (to_sq % 8 == 0)) break;
            to_sq += offset;
            if (to_sq < 0 || to_sq > 63) break;
            
            Square t = static_cast<Square>(to_sq);
            if (get_bit(board.pieces[static_cast<uint8_t>(rook)], t) || get_bit(board.pieces[static_cast<uint8_t>(queen)], t)) return true;
            if (get_bit(total_occ, t)) break; 
        }
    }

    return false;
}

// We must explicitly instantiate the templates at the bottom of the file for no linker issues
template MoveList generate_moves<Color::White>(const BoardState& board) noexcept;
template MoveList generate_moves<Color::Black>(const BoardState& board) noexcept;

template bool is_square_attacked<Color::White>(Square sq, const BoardState& board) noexcept;
template bool is_square_attacked<Color::Black>(Square sq, const BoardState& board) noexcept;