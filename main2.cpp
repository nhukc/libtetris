#include "bot/MoveGen.hpp"
#include "bot/Analysis.hpp"
#include "util/Fumen.hpp"
#include <iostream>

#ifdef EMSCRIPTEN
#include "emscripten.h"

extern "C" {
EMSCRIPTEN_KEEPALIVE
int versionabc() {
    return 5;
}
}
#endif

int main() {
    tetris::Board board(10, 20);
    AnalysisContext c;

    std::cout << "Enter fumen: ";
    std::string fumen_input;
    std::cin >> fumen_input;
    Fumen fumen = decode(fumen_input);
    tetris::Board fumen_board = fumen.pages[fumen.pages.size() - 1];
    std::cout << fumen_board << "\n";
    
    std::cout << "Enter piece sequence: ";
    std::string piece_sequence;
    std::cin >> piece_sequence;

    std::cout << "Enter depth: ";
    int x;
    std::cin >> x;

    for(int i = 0; i < 10; i++) {
        for(int j = 0; j < 20; j++) {
            board.FillSquare(i, j, fumen_board.GetSquare(i, j+1));
        }
    }

    std::vector<tetris::Tile> minos;
    for(char& c : piece_sequence) {
        switch(c) {
            case 'I':
                minos.push_back(tetris::Tile::I);
                break;
            case 'O':
                minos.push_back(tetris::Tile::O);
                break;
            case 'L':
                minos.push_back(tetris::Tile::L);
                break;
            case 'J':
                minos.push_back(tetris::Tile::J);
                break;
            case 'S':
                minos.push_back(tetris::Tile::S);
                break;
            case 'Z':
                minos.push_back(tetris::Tile::Z);
                break;
            case 'T':
                minos.push_back(tetris::Tile::T);
                break;
            default:
                assert("Invalid piece type\n");
        }
    }

    c.IterativeDeepen(x, ToBitBoard(board), minos);
    std::cout << "Score: " << c.best_score << "\n";
    std::cout << "Moves: " << c.best_moves.size() << "\n";
    for(auto move : c.best_moves) {
        board.ApplyMino(*FromMoveInfo(move));
        tetris::Board temp = board;
        board.UpdateBoard();
        bool print = false;
        for(int i = 0; i < 10; i++) {
            for(int j = 0; j < 20; j++) {
                if(board.GetSquare(i,j) != temp.GetSquare(i,j))
                    print = true;
            }  
        }
        if(print)
            std::cout << temp << "\n";
        std::cout << board << "\n";
    }
    std::cout << board << "\n";
    std::cout << ToBitBoard(board) << "\n";
}
