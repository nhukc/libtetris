#include "bot/MoveGen.hpp"
#include <iostream>

int main() {
    tetris::Board board(10, 20);
    tetris::IMino i1(board);
    std::vector<move_info> a = FindPossiblePositions(ToBitBoard(board), i1.GetTile());
    std::vector<move_info> b;
    for(auto& mino : a) {
        tetris::LMino l1(board);
        tetris::Board temp_board = board;
        temp_board.ApplyMino(*FromMoveInfo(mino));
        std::vector<move_info> tmp = FindPossiblePositions(ToBitBoard(temp_board), l1.GetTile());
        for(auto& x : tmp) {
            b.push_back(std::move(x));
        }
        tmp.clear();
    }
    a.clear();
    for(auto& x : b) {
        a.push_back(std::move(x));
    }
    std::cout << b.size() << "\n";
    b.clear();
    for(auto& mino : a) {
        tetris::TMino l1(board);
        tetris::Board temp_board = board;
        temp_board.ApplyMino(*FromMoveInfo(mino));
        std::vector<move_info> tmp = FindPossiblePositions(ToBitBoard(temp_board), l1.GetTile());
        for(auto& x : tmp) {
            b.push_back(std::move(x));
        }
        tmp.clear();
    }
    a.clear();
    for(auto& x : b) {
        a.push_back(std::move(x));
    }
    std::cout << b.size() << "\n";
    b.clear();
    for(auto& mino : a) {
        tetris::OMino l1(board);
        tetris::Board temp_board = board;
        temp_board.ApplyMino(*FromMoveInfo(mino));
        std::vector<move_info> tmp = FindPossiblePositions(ToBitBoard(temp_board), l1.GetTile());
        for(auto& x : tmp) {
            b.push_back(std::move(x));
        }
        tmp.clear();
    }
    a.clear();
    for(auto& x : b) {
        a.push_back(std::move(x));
    }
    std::cout << b.size() << "\n";
    b.clear();
}
