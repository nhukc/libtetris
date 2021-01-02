#include "util/Fumen.hpp"
#include "bot/MoveGen.hpp"
#include <assert.h>
#include <iostream>

int main() {
    Fumen f;
    f = decode("v115@vhKkMJ9tBHjByuBDkBmfBplBNrBkmBHsBAAA");
    tetris::Board b = f.pages[8];
    b.UpdateBoard();
    tetris::SMino mino(b);
    auto vec = FindPossiblePositions(ToBitBoard(b), mino.GetTile());

    for(auto& x : vec) {
        tetris::Board b2 = b;
        b2.ApplyMino(*FromMoveInfo(x));
        std::cout << b2 << "\n";
    }
    std::cout << vec.size();
    assert(vec.size() == 18);

}
