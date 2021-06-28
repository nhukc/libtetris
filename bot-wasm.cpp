#include "bot/MoveGen.hpp"
#include "bot/Analysis.hpp"
#include "util/Fumen.hpp"
#include <iostream>

#ifdef EMSCRIPTEN
#include "emscripten.h"

extern "C" {
    EMSCRIPTEN_KEEPALIVE
    uint8_t* AllocateMinoList(int max_mino) {
        return (uint8_t *)malloc(max_mino*4*sizeof(uint8_t));
    }
}

extern "C" {
    EMSCRIPTEN_KEEPALIVE
    void FreeMinoList(uint8_t* p) {
        free(p);
    }
}

extern "C" {
    EMSCRIPTEN_KEEPALIVE
    int OutputSize(int max_mino) {
        return max_mino*4*sizeof(uint8_t);
    }
}

extern "C" {
    EMSCRIPTEN_KEEPALIVE
    void AnalyzeFumen(char * fumenjs, char * piecesjs, int depth, uint8_t* minoresultjs) {
        tetris::Board board(10, 20);
        AnalysisContext c;
        std::string fumen_input(fumenjs);
        std::string piece_sequence(piecesjs);
        
        Fumen fumen = decode(fumen_input);
        tetris::Board fumen_board = fumen.pages[fumen.pages.size() - 1];
    
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
        c.IterativeDeepen(depth, ToBitBoard(board), minos);
        for(int i = 0; i < c.best_moves.size(); i++) {
            move_info& move = c.best_moves[i];
            minoresultjs[i*4] = ToInt(move.piece_type);
            minoresultjs[i*4+1] = move.x;
            minoresultjs[i*4+2] = move.y;
            minoresultjs[i*4+3] = move.orientation;
        }
    }
}
#endif
