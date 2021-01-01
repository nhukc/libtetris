#include "core/Tetris.hpp"
#include <assert.h>
#include <iostream>

int main() {
    tetris::Board b(10,22);
    
    tetris::IMino i1(b);
    i1.Rotate(tetris::Rotation::Clockwise, b);
    i1.Translate(0,-18, b);
    b.ApplyMino(i1);
    std::cout << i1.x << " " << i1.y << "\n";
    std::cout << b << "\n";    
    
    tetris::LMino l1(b);
    l1.Translate(4,-20, b);
    b.ApplyMino(l1);
    std::cout << l1.x << " " << l1.y << "\n";
    std::cout << b << "\n";    
    
    tetris::TMino t1(b);
    t1.Rotate(tetris::Rotation::Clockwise, b);
    t1.Rotate(tetris::Rotation::Clockwise, b);
    t1.Translate(4,-18, b);
    b.ApplyMino(t1);
    std::cout << t1.x << " " << t1.y << "\n";
    std::cout << b << "\n";    
    
    tetris::OMino o1(b);
    o1.Translate(1,-16, b);
    b.ApplyMino(o1);
    std::cout << o1.x << " " << o1.y << "\n";
    std::cout << b << "\n";    
    
    tetris::TMino t2(b);
    t2.Translate(3,-17, b);
    tetris::RotationContext r = t2.Rotate(tetris::Rotation::Clockwise, b);
    assert(r.kicked);
    b.ApplyMino(t2);
    std::cout << t2.x << " " << t2.y << "\n";
    std::cout << b << "\n";    
}
