#include "UCI.hpp"
#include "Search.hpp"
#include "MoveGen.hpp"
#include <iostream>
#include <sstream>
#include <cctype>

BoardState parse_fen(const std::string& fen) {
    BoardState board; // all 0 rn
    
    // std::istringstream splits the string by spaces
    std::istringstream iss(fen);
    std::string pieces, color, castling, en_passant;
    iss>>pieces>>color>>castling>>en_passant;

    // Parde pieces
    int rank = 7; // Rank 8 (Index 7)
    int file = 0; // File A (Index 0)

    for (char c : pieces) {
        if (c == '/') {
            rank--;   // Move down a row
            file = 0; // Reset to the A file
        } else if (isdigit(c)) {
            file += (c - '0'); // Skip empty squares
        } else {
            Square sq = static_cast<Square>(rank * 8 + file);
            int piece = -1;
            
            // Match the letter to our PieceType enum
            switch (c) {
                case 'P': piece = static_cast<int>(PieceType::WhitePawn); break;
                case 'N': piece = static_cast<int>(PieceType::WhiteKnight); break;
                case 'B': piece = static_cast<int>(PieceType::WhiteBishop); break;
                case 'R': piece = static_cast<int>(PieceType::WhiteRook); break;
                case 'Q': piece = static_cast<int>(PieceType::WhiteQueen); break;
                case 'K': piece = static_cast<int>(PieceType::WhiteKing); break;
                case 'p': piece = static_cast<int>(PieceType::BlackPawn); break;
                case 'n': piece = static_cast<int>(PieceType::BlackKnight); break;
                case 'b': piece = static_cast<int>(PieceType::BlackBishop); break;
                case 'r': piece = static_cast<int>(PieceType::BlackRook); break;
                case 'q': piece = static_cast<int>(PieceType::BlackQueen); break;
                case 'k': piece = static_cast<int>(PieceType::BlackKing); break;
            }
            
            // Set the bit on the correct board
            if (piece != -1) set_bit(board.pieces[piece], sq);
            file++;
        }
    }

    // 2. REBUILD OCCUPANCIES
    for (int i = 0; i < 6; i++) board.occupancies[static_cast<uint8_t>(Color::White)] |= board.pieces[i];
    for (int i = 6; i < 12; i++) board.occupancies[static_cast<uint8_t>(Color::Black)] |= board.pieces[i];
    board.occupancies[static_cast<uint8_t>(Color::Both)] = 
        board.occupancies[static_cast<uint8_t>(Color::White)] | board.occupancies[static_cast<uint8_t>(Color::Black)];

    // 3. PARSE CASTLING RIGHTS
    board.castle_rights.rights = 0;
    for (char c : castling) {
        if (c == 'K') board.castle_rights.rights |= CastleRights::WK;
        if (c == 'Q') board.castle_rights.rights |= CastleRights::WQ;
        if (c == 'k') board.castle_rights.rights |= CastleRights::BK;
        if (c == 'q') board.castle_rights.rights |= CastleRights::BQ;
    }

    // 4. PARSE EN PASSANT (Optional safety for starting position)
    if (en_passant != "-") {
        int file_idx = en_passant[0] - 'a';
        int rank_idx = en_passant[1] - '1';
        board.en_passant_square = static_cast<Square>(rank_idx * 8 + file_idx);
    }

    return board;
}




// Helper function: Translates a Move object to UCI text (e.g., "e2e4")
// Helper function: Translates a Move object to UCI text (e.g., "e2e4")
std::string move_to_string(const Move& m) {
    int src = static_cast<int>(m.get_start());
    int tgt = static_cast<int>(m.get_end());

    std::string s = "";
    s += static_cast<char>('a' + (src % 8));
    s += static_cast<char>('1' + (src / 8));
    s += static_cast<char>('a' + (tgt % 8));
    s += static_cast<char>('1' + (tgt / 8));

    uint16_t flag = m.get_flag();
    if (flag == FLAG_PROMO_QUEEN)  s += 'q';
    else if (flag == FLAG_PROMO_ROOK)   s += 'r';
    else if (flag == FLAG_PROMO_BISHOP) s += 'b';
    else if (flag == FLAG_PROMO_KNIGHT) s += 'n';

    return s;
}
// The Infinite Listener Loop
void uci_loop() {
    std::string line;
    // Default to the standard starting board
    BoardState current_board = parse_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    Color current_turn = Color::White;

    while (std::getline(std::cin, line)) {
        std::istringstream iss(line);
        std::string token;
        iss >> token;

        // 1. Identification
        if (token == "uci") {
            std::cout << "id name HFT_Engine_v1" << std::endl;
            std::cout << "id author You" << std::endl;
            std::cout << "uciok" << std::endl;
        } 
        // 2. Readiness Check
        else if (token == "isready") {
            std::cout << "readyok" << std::endl;
        } 
        // 3. Setup the Board
        // 3. Setup the Board
        else if (token == "position") {
            std::string type;
            iss >> type;
            
            if (type == "startpos") {
                current_board = parse_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
                current_turn = Color::White;
            } else if (type == "fen") {
                std::string fen_str = "", fen_part;
                while (iss >> fen_part && fen_part != "moves") {
                    fen_str += fen_part + " ";
                    if (fen_part == "w") current_turn = Color::White;
                    if (fen_part == "b") current_turn = Color::Black;
                }
                current_board = parse_fen(fen_str);
            }
            
            // --- THE FIX: PARSE THE MOVE HISTORY ---
            std::string word;
            while (iss >> word) {
                if (word == "moves") continue;
                
                // Generate all legal moves for the current turn
                MoveList mvl;
                if (current_turn == Color::White) {
                    mvl = generate_moves<Color::White>(current_board);
                } else {
                    mvl = generate_moves<Color::Black>(current_board);
                }

                // Find the move that matches the text Arena sent (e.g., "e2e4")
                for (size_t i = 0; i < mvl.size(); i++) {
                    if (move_to_string(mvl.begin()[i]) == word) {
                        // Play the move on our board and flip the turn!
                        current_board = current_board.make_move(mvl.begin()[i]);
                        current_turn = (current_turn == Color::White) ? Color::Black : Color::White;
                        break;
                    }
                }
            }
        }
        // 4. Calculate and Play!
        else if (token == "go") {
            int depth = 5; // Default depth if GUI doesn't specify
            std::string param;
            while (iss >> param) {
                if (param == "depth") iss >> depth;
            }

            Move best_move;
            if (current_turn == Color::White) {
                best_move = get_best_move<Color::White>(current_board, depth);
            } else {
                best_move = get_best_move<Color::Black>(current_board, depth);
            }

            std::cout << "bestmove " << move_to_string(best_move) << std::endl;
        } 
        // 5. Exit
        else if (token == "quit") {
            break;
        }
    }
}