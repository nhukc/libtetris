#include "bot/MoveGen.hpp"
#include "core/Tetris.hpp"
#include <assert.h>
#include <iostream>

int main() {
    tetris::Board b(10,22);
    
    tetris::ZMino mino(b);
    auto vec = FindPossiblePositions(b, mino);

    for(auto& x : vec) {
        tetris::Board b2(10, 22);
        b2.ApplyMino(*x);
        std::cout << b2 << "\n";
    }
    std::cout << vec.size();
    assert(vec.size() == 17);
}
