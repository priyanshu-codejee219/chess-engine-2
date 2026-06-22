#include "Eval.hpp"

#define MIRROR(sq) ((sq) ^ 56)

const int PawnPST[64] = {
      0,  0,  0,  0,  0,  0,  0,  0,
     50, 50, 50, 50, 50, 50, 50, 50,
     10, 10, 20, 30, 30, 20, 10, 10,
      5,  5, 10, 25, 25, 10,  5,  5,
      0,  0,  0, 20, 20,  0,  0,  0,
      5, -5,-10,  0,  0,-10, -5,  5,
      5, 10, 10,-20,-20, 10, 10,  5,
      0,  0,  0,  0,  0,  0,  0,  0
};

const int KnightPST[64] = {
    -50,-40,-30,-30,-30,-30,-40,-50,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50
};

const int BishopPST[64] = {
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0, 10, 10, 10, 10,  0,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -20,-10,-10,-10,-10,-10,-10,-20
};

const int RookPST[64] = {
      0,  0,  0,  0,  0,  0,  0,  0,
      5, 10, 10, 10, 10, 10, 10,  5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
      0,  0,  0,  5,  5,  0,  0,  0
};

const int QueenPST[64] = {
    -20,-10,-10, -5, -5,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5,  5,  5,  5,  0,-10,
     -5,  0,  5,  5,  5,  5,  0, -5,
      0,  0,  5,  5,  5,  5,  0, -5,
    -10,  5,  5,  5,  5,  5,  0,-10,
    -10,  0,  5,  0,  0,  0,  0,-10,
    -20,-10,-10, -5, -5,-10,-10,-20
};

const int KingMidgamePST[64] = {
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -20,-30,-30,-40,-40,-30,-30,-20,
    -10,-20,-20,-20,-20,-20,-20,-10,
     20, 20,  0,  0,  0,  0, 20, 20,
     20, 30, 10,  0,  0, 10, 30, 20
};

const int KingEndgamePST[64] = {
    -50,-40,-30,-20,-20,-30,-40,-50,
    -30,-20,-10,  0,  0,-10,-20,-30,
    -30,-10, 20, 30, 30, 20,-10,-30,
    -30,-10, 30, 40, 40, 30,-10,-30,
    -30,-10, 30, 40, 40, 30,-10,-30,
    -30,-10, 20, 30, 30, 20,-10,-30,
    -30,-30,  0,  0,  0,  0,-30,-30,
    -50,-30,-30,-30,-30,-30,-30,-50
};

const int PawnPSTEndgame[64] = {
      0,  0,  0,  0,  0,  0,  0,  0,
     80, 80, 80, 80, 80, 80, 80, 80,
     50, 50, 50, 50, 50, 50, 50, 50,
     30, 30, 30, 30, 30, 30, 30, 30,
     20, 20, 20, 20, 20, 20, 20, 20,
     10, 10, 10, 10, 10, 10, 10, 10,
     10, 10, 10, 10, 10, 10, 10, 10,
      0,  0,  0,  0,  0,  0,  0,  0
};

//get number of pieces on the board
inline int popcount(Bitboard b) noexcept {
    return __builtin_popcountll(b);
}

// Phase weights — standard "non-pawn material" tapering scheme.
constexpr int KNIGHT_PHASE = 1;
constexpr int BISHOP_PHASE = 1;
constexpr int ROOK_PHASE   = 2;
constexpr int QUEEN_PHASE  = 4;
constexpr int TOTAL_PHASE  = (KNIGHT_PHASE * 4) + (BISHOP_PHASE * 4) +
                              (ROOK_PHASE * 4) + (QUEEN_PHASE * 2); // = 24

inline int compute_phase(const BoardState& board) noexcept {
    int phase = 0;
    phase += popcount(board.pieces[static_cast<uint8_t>(PieceType::WhiteKnight)]) * KNIGHT_PHASE;
    phase += popcount(board.pieces[static_cast<uint8_t>(PieceType::BlackKnight)]) * KNIGHT_PHASE;
    phase += popcount(board.pieces[static_cast<uint8_t>(PieceType::WhiteBishop)]) * BISHOP_PHASE;
    phase += popcount(board.pieces[static_cast<uint8_t>(PieceType::BlackBishop)]) * BISHOP_PHASE;
    phase += popcount(board.pieces[static_cast<uint8_t>(PieceType::WhiteRook)])   * ROOK_PHASE;
    phase += popcount(board.pieces[static_cast<uint8_t>(PieceType::BlackRook)])   * ROOK_PHASE;
    phase += popcount(board.pieces[static_cast<uint8_t>(PieceType::WhiteQueen)])  * QUEEN_PHASE;
    phase += popcount(board.pieces[static_cast<uint8_t>(PieceType::BlackQueen)])  * QUEEN_PHASE;

    // Clamp in case of some bizarre promotion edge case giving extra majors.
    if (phase > TOTAL_PHASE) phase = TOTAL_PHASE;
    return phase;
}



template<Color side>
int evaluate(const BoardState& board) noexcept{
    int white_score = 0;
    int black_score = 0;

    int phase = compute_phase(board); 

    // White pawns (tapered)
    {
        Bitboard bb = board.pieces[static_cast<uint8_t>(PieceType::WhitePawn)];
        while (bb) {
            int sq = __builtin_ctzll(bb);
            int mg = PawnPST[MIRROR(sq)];
            int eg = PawnPSTEndgame[MIRROR(sq)];
            int tapered = (mg * phase + eg * (TOTAL_PHASE - phase)) / TOTAL_PHASE;
            white_score += PAWN_VAL + tapered;
            bb &= bb - 1;
        }
    }

    // White knights
    {
        Bitboard bb = board.pieces[static_cast<uint8_t>(PieceType::WhiteKnight)];
        while (bb) {
            int sq = __builtin_ctzll(bb);
            white_score += KNIGHT_VAL + KnightPST[MIRROR(sq)];
            bb &= bb - 1;
        }
    }

    // White bishops
    {
        Bitboard bb = board.pieces[static_cast<uint8_t>(PieceType::WhiteBishop)];
        while (bb) {
            int sq = __builtin_ctzll(bb);
            white_score += BISHOP_VAL + BishopPST[MIRROR(sq)];
            bb &= bb - 1;
        }
    }

    // White rooks
    {
        Bitboard bb = board.pieces[static_cast<uint8_t>(PieceType::WhiteRook)];
        while (bb) {
            int sq = __builtin_ctzll(bb);
            white_score += ROOK_VAL + RookPST[MIRROR(sq)];
            bb &= bb - 1;
        }
    }

    // White queen
    {
        Bitboard bb = board.pieces[static_cast<uint8_t>(PieceType::WhiteQueen)];
        while (bb) {
            int sq = __builtin_ctzll(bb);
            white_score += QUEEN_VAL + QueenPST[MIRROR(sq)];
            bb &= bb - 1;
        }
    }

    // Black pawns (tapered)
    {
        Bitboard bb = board.pieces[static_cast<uint8_t>(PieceType::BlackPawn)];
        while (bb) {
            int sq = __builtin_ctzll(bb);
            int mg = PawnPST[sq];
            int eg = PawnPSTEndgame[sq];
            int tapered = (mg * phase + eg * (TOTAL_PHASE - phase)) / TOTAL_PHASE;
            black_score += PAWN_VAL + tapered;
            bb &= bb - 1;
        }
    }

    // Black knights
    {
        Bitboard bb = board.pieces[static_cast<uint8_t>(PieceType::BlackKnight)];
        while (bb) {
            int sq = __builtin_ctzll(bb);
            black_score += KNIGHT_VAL + KnightPST[sq];
            bb &= bb - 1;
        }
    }

    // Black bishops
    {
        Bitboard bb = board.pieces[static_cast<uint8_t>(PieceType::BlackBishop)];
        while (bb) {
            int sq = __builtin_ctzll(bb);
            black_score += BISHOP_VAL + BishopPST[sq];
            bb &= bb - 1;
        }
    }

    // Black rooks
    {
        Bitboard bb = board.pieces[static_cast<uint8_t>(PieceType::BlackRook)];
        while (bb) {
            int sq = __builtin_ctzll(bb);
            black_score += ROOK_VAL + RookPST[sq];
            bb &= bb - 1;
        }
    }

    // Black queen
    {
        Bitboard bb = board.pieces[static_cast<uint8_t>(PieceType::BlackQueen)];
        while (bb) {
            int sq = __builtin_ctzll(bb);
            black_score += QUEEN_VAL + QueenPST[sq];
            bb &= bb - 1;
        }
    }

    // White king (tapered)
    {
        Bitboard bb = board.pieces[static_cast<uint8_t>(PieceType::WhiteKing)];
        while (bb) {
            int sq = __builtin_ctzll(bb);
            int mg = KingMidgamePST[MIRROR(sq)];
            int eg = KingEndgamePST[MIRROR(sq)];
            int tapered = (mg * phase + eg * (TOTAL_PHASE - phase)) / TOTAL_PHASE;
            white_score += KING_VAL + tapered;
            bb &= bb - 1;
        }
    }

    // Black king (tapered)
    {
        Bitboard bb = board.pieces[static_cast<uint8_t>(PieceType::BlackKing)];
        while (bb) {
            int sq = __builtin_ctzll(bb);
            int mg = KingMidgamePST[sq];
            int eg = KingEndgamePST[sq];
            int tapered = (mg * phase + eg * (TOTAL_PHASE - phase)) / TOTAL_PHASE;
            black_score += KING_VAL + tapered;
            bb &= bb - 1;
        }
    }

    if constexpr(side == Color::White) {
        return white_score - black_score;
    } else {
        return black_score - white_score;
    }
}

//template instantiation for linker to find it
template int evaluate<Color::White>(const BoardState& board) noexcept;
template int evaluate<Color::Black>(const BoardState& board) noexcept;