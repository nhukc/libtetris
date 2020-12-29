#include "core/Tetris.hpp"
#include <assert.h>
#include <iostream>

int main() {
    tetris::Board b(10,22);
    
    tetris::IMino i1(b);
    i1.Rotate(tetris::Rotation::Clockwise);
    i1.Translate(0,-18);
    i1.ApplyToBoard();
    std::cout << i1.x << " " << i1.y << "\n";
    std::cout << b << "\n";    
    
    tetris::LMino l1(b);
    l1.Translate(4,-20);
    l1.ApplyToBoard();
    std::cout << l1.x << " " << l1.y << "\n";
    std::cout << b << "\n";    
    
    tetris::TMino t1(b);
    t1.Rotate(tetris::Rotation::Clockwise);
    t1.Rotate(tetris::Rotation::Clockwise);
    t1.Translate(4,-18);
    t1.ApplyToBoard();
    std::cout << t1.x << " " << t1.y << "\n";
    std::cout << b << "\n";    
    
    tetris::OMino o1(b);
    o1.Translate(1,-16);
    o1.ApplyToBoard();
    std::cout << o1.x << " " << o1.y << "\n";
    std::cout << b << "\n";    
    
    tetris::TMino t2(b);
    t2.Translate(3,-17);
    tetris::RotationContext r = t2.Rotate(tetris::Rotation::Clockwise);
    assert(r.kicked);
    t2.ApplyToBoard();
    std::cout << t2.x << " " << t2.y << "\n";
    std::cout << b << "\n";    
}
