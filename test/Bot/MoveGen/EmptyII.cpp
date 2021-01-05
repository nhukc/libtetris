#include "bot/MoveGen.hpp"
#include "core/Tetris.hpp"
#include "util/Fumen.hpp"
#include <assert.h>
#include <iostream>

int main() {
    tetris::Board fumen = decode("v115@vhEhOJhpBhwBhrBAAA").pages[0];
    tetris::Board b(10, 22);
    for(int i = 0; i < 10; i++) {
        for(int j = 0; j < 22; j++) {
            b.FillSquare(i, j, fumen.GetSquare(i, j+1));
        }
    }
    
    tetris::IMino mino(b);
    auto vec = FindPossiblePositions(ToBitBoard(b), mino.GetTile());

    for(auto& x : vec) {
        tetris::Board b2 = b;
        b2.ApplyMino(*FromMoveInfo(x));
        std::cout << b2 << "\n";
    }
    std::cout << vec.size();
}
