#include "Search.hpp"
#include "MoveGen.hpp"
#include "Eval.hpp"
#include<chrono>
#include<iostream>
#include<algorithm>



constexpr int MVV_LVA_SCORES[12] = {
    100, 300, 300, 500, 900, 10000, 
    100, 300, 300, 500, 900, 10000
};

// return the score of the piece at a given square.
inline int get_piece_score(const BoardState& board, Square sq){
    for (int i = 0; i < 12; i++) {
        if (get_bit(board.pieces[i], sq)) {
            return MVV_LVA_SCORES[i];
        }
    }
    return 0; //fallback to enpty square/
}


inline bool is_capture(const BoardState& board, Move move) {
    return get_piece_score(board, move.get_end()) > 0;
}


inline int score_move(const BoardState& board, Move move) {
    int attacker_score = get_piece_score(board, move.get_start());
    int victim_score = get_piece_score(board, move.get_end());

    if (victim_score > 0) {
        return (victim_score * 10) - attacker_score;
    }
    return 0; // assume non capture a score of 0.
}

long long nodes_searched = 0;

template<Color side>
[[nodiscard]] int quiescence_search(const BoardState& board, int alpha, int beta, int ply) noexcept {
    nodes_searched++;

    constexpr Color next_side = (side == Color::White) ? Color::Black : Color::White;
    constexpr PieceType my_king = (side == Color::White) ? PieceType::WhiteKing : PieceType::BlackKing;

    int king_sq = __builtin_ctzll(board.pieces[static_cast<uint8_t>(my_king)]);
    bool in_check = is_square_attacked<next_side>(static_cast<Square>(king_sq), board);

    // Only trust the static eval as a floor if we are NOT in check.
    // A side in check has no "safe" stand-pat score - it might be getting mated.
    if (!in_check) {
        int stand_pat = evaluate<side>(board);
        if (stand_pat >= beta) return beta;
        if (alpha < stand_pat) alpha = stand_pat;
    }

    MoveList mvl = generate_moves<side>(board);

    // If in check, search ALL moves (evasions). Otherwise, only captures.
    MoveList search_list;
    for (size_t i = 0; i < mvl.size(); i++) {
        Move move = mvl.begin()[i];
        if (in_check || is_capture(board, move)) {
            search_list.push(move);
        }
    }

    std::sort(search_list.begin(), search_list.end(), [&board](Move a, Move b) {
        return score_move(board, a) > score_move(board, b);
    });

    int legal_moves = 0;

    for (size_t i = 0; i < search_list.size(); i++) {
        const Move mv = search_list.begin()[i];
        BoardState next_state = board.make_move(mv);

        int pos_my_king = __builtin_ctzll(next_state.pieces[static_cast<uint8_t>(my_king)]);
        bool isa = is_square_attacked<next_side>(static_cast<Square>(pos_my_king), next_state);
        if (isa) continue;

        legal_moves++;

        int score = -1 * quiescence_search<next_side>(next_state, -beta, -alpha, ply + 1);

        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }

    // No legal evasions while in check -> checkmate inside qsearch.
    if (in_check && legal_moves == 0) {
        return -MATE_VALUE + ply;
    }

    return alpha;
}

template<Color side>
[[nodiscard]] int search(const BoardState& bst, int depth, int alpha, int beta, int ply) noexcept{
    nodes_searched++;

    if(depth==0) return quiescence_search<side>(bst, alpha, beta, ply);
    //inti score    
    int best_score = -INF;
    int legal_moves = 0;

    //get all possibel moves from this staet
    MoveList mvl = generate_moves<side>(bst);
    std::sort(mvl.begin(), mvl.end(), [&bst](Move a, Move b) {
        return score_move(bst, a) > score_move(bst, b);
    });

    constexpr Color next_side = (side==Color::White)?Color::Black:Color::White;

    //loopthroguh every possiblw move
    for(size_t i=0; i<mvl.size(); i++){
        const Move mv = mvl.begin()[i];

        BoardState next_state = bst.make_move(mv);

        //check if the piece moved was a pinned one.
        constexpr PieceType my_king = (side==Color::White?PieceType::WhiteKing:PieceType::BlackKing);
        int pos_my_king = __builtin_ctzll(next_state.pieces[static_cast<uint8_t>(my_king)]);
        bool isa = is_square_attacked<next_side>(static_cast<Square>(pos_my_king), next_state);
        if(isa) continue;
        else legal_moves++;

        int score = -1*search<next_side>(next_state, depth-1, -beta, -alpha, ply+1);
        if(score > best_score) best_score = score;
        if(score > alpha) alpha = score;
        if(alpha >= beta) break;
    }

    //chesk for checkmate or stalemate
    if(legal_moves==0){
        constexpr PieceType my_king = (side==Color::White?PieceType::WhiteKing:PieceType::BlackKing);
        int pos_my_king = __builtin_ctzll(bst.pieces[static_cast<uint8_t>(my_king)]);
        bool isa = is_square_attacked<next_side>(static_cast<Square>(pos_my_king), bst);
        if(isa) return -MATE_VALUE + ply; //checkmate
        else return 0; //stalemate
    }

    return best_score;

}




template<Color side>
[[nodiscard]] Move get_best_move(const BoardState& bst, int depth) noexcept {
    nodes_searched=0;
    auto start_time = std::chrono::high_resolution_clock::now();

    MoveList mvl = generate_moves<side>(bst);
    constexpr Color next_side = (side == Color::White) ? Color::Black : Color::White;
    
    Move best_move;
    // Fallback: Default to the very first generated move just in case
    if (mvl.size() > 0) best_move = mvl.begin()[0]; 
    
    int best_score = -INF;
    int alpha = -INF;
    int beta = INF;

    for (size_t i = 0; i < mvl.size(); i++) {
        const Move mv = mvl.begin()[i];
        BoardState next_state = bst.make_move(mv);
        
        // Legality Check
        constexpr PieceType my_king = (side == Color::White) ? PieceType::WhiteKing : PieceType::BlackKing;
        int pos_my_king = __builtin_ctzll(next_state.pieces[static_cast<uint8_t>(my_king)]);
        bool isa = is_square_attacked<next_side>(static_cast<Square>(pos_my_king), next_state);
        
        if (isa) continue;

        // The Negamax Recursion
        int score = -1 * search<next_side>(next_state, depth - 1, -beta, -alpha, 1);

        // Update the best move if we found a better score
        if (score > best_score) {
            best_score = score;
            best_move = mv; 
        }
        if (score > alpha) {
            alpha = score;
        }
        // Note: We don't technically need the "alpha >= beta break" at the root 
        // since beta is starting at INF, but standard structure keeps it clean.
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    if(duration==0) duration = 1;
    long long nps = (nodes_searched*1000)/duration;

    std::cout << "info depth " << depth << " nodes " << nodes_searched << " time " << duration << " nps " << nps << std::endl;
    
    return best_move;
}

template Move get_best_move<Color::White>(const BoardState& bst, int depth) noexcept;
template Move get_best_move<Color::Black>(const BoardState& bst, int depth) noexcept;

template int quiescence_search<Color::White>(const BoardState& board, int alpha, int beta, int ply) noexcept;
template int quiescence_search<Color::Black>(const BoardState& board, int alpha, int beta, int ply) noexcept;

template int search<Color::White> (const BoardState& bst, int depth, int alpha, int beta, int ply) noexcept;
template int search<Color::Black> (const BoardState& bst, int depth, int alpha, int beta, int ply) noexcept;