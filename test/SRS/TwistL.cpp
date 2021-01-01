#include "core/Tetris.hpp"
#include <assert.h>
#include <iostream>

int main() {
    tetris::Board b(10,22);
    
    tetris::IMino i1(b);
    i1.Translate(0,-20, b);
    b.ApplyMino(i1);
    std::cout << i1.x << " " << i1.y << "\n";
    std::cout << b << "\n";
    
    tetris::IMino i2(b);
    i2.Translate(2,-19, b);
    b.ApplyMino(i2);
    std::cout << i2.x << " " << i2.y << "\n";
    std::cout << b << "\n";
    
    tetris::LMino l1(b);
    l1.Rotate(tetris::Rotation::CounterClockwise, b);
    l1.Translate(5,-19, b);
    tetris::RotationContext r = l1.Rotate(tetris::Rotation::Clockwise, b);
    assert(r.kicked);
    b.ApplyMino(l1);
    std::cout << l1.x << " " << l1.y << "\n";
    std::cout << b << "\n";    
}
