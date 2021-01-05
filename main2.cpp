#include "bot/MoveGen.hpp"
#include "bot/Analysis.hpp"
#include <iostream>

int main() {
    tetris::Board board(10, 20);
    AnalysisContext c;
    int x;
    std::cin >> x;
    c.IterativeDeepen(x, ToBitBoard(board), {tetris::Tile::I, tetris::Tile::I, tetris::Tile::I, tetris::Tile::I, tetris::Tile::O});
    std::cout << c.best_score << "\n";
    std::cout << c.best_moves.size() << "\n";
    for(auto move : c.best_moves) {
        board.ApplyMino(*FromMoveInfo(move));
    }
    std::cout << board << "\n";
    std::cout << ToBitBoard(board) << "\n";
}
