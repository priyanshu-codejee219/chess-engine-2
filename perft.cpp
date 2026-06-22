#include "Board.hpp"
#include "MoveGen.hpp"
#include "MoveList.hpp"
#include "UCI.hpp"
#include <iostream>
#include <chrono>

// Quick helper to check if a king was captured (meaning the previous move was illegal)
// In a full engine, we write a dedicated 'is_in_check' function, but this works for testing.
inline bool king_captured(const BoardState& board, Color side) {
    PieceType king_type = (side == Color::White) ? PieceType::WhiteKing : PieceType::BlackKing;
    return board.pieces[static_cast<uint8_t>(king_type)] == 0;
}

template<Color side>
uint64_t perft(const BoardState& board, int depth) {
    // Base case: we reached the leaf node
    if (depth == 0) return 1ULL;

    uint64_t nodes = 0;
    MoveList moves = generate_moves<side>(board);
    constexpr Color next_side = (side == Color::White) ? Color::Black : Color::White;

    for (int i = 0; i < moves.size(); ++i) {
        BoardState next_state = board.make_move(moves.begin()[i]);
        
        // Find OUR King's square on the newly updated board
        PieceType my_king = (side == Color::White) ? PieceType::WhiteKing : PieceType::BlackKing;
        int king_sq = __builtin_ctzll(next_state.pieces[static_cast<uint8_t>(my_king)]);

        // If the enemy attacks our King after we move, our move was illegal! Snap the branch.
        if (is_square_attacked<next_side>(static_cast<Square>(king_sq), next_state)) {
            continue;
        }

        nodes += perft<next_side>(next_state, depth - 1);
    }  

    return nodes;
}

// Explicit template instantiations
template uint64_t perft<Color::White>(const BoardState& board, int depth);
template uint64_t perft<Color::Black>(const BoardState& board, int depth);

void run_perft_benchmark(int depth){
    BoardState starting_board = parse_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - o 1");
    std::cout<<"Starting Perft Benchmark to Depth "<<depth<<"...\n";

    auto start_time = std::chrono::high_resolution_clock::now();

    uint64_t total_nodes = perft<Color::White>(starting_board, depth);

    auto end_time = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    if (ms == 0) ms = 1; 

    //MNPS
    double seconds = ms / 1000.0;
    double mnps = (total_nodes / 1000000.0) / seconds;

    std::cout << "--- PERFT RESULTS ---" << "\n";
    std::cout << "Total Nodes: " << total_nodes << "\n";
    std::cout << "Time Taken:  " << ms << " ms\n";
    std::cout << "Speed:       " << mnps << " MNPS\n";
    std::cout << "--------------------\n";
}