#include "core/Tetris.hpp"
#include <assert.h>
#include <iostream>

int main() {
    tetris::Board b(10,22);
    
    tetris::IMino i1(b);
    i1.Translate(-3,-20, b);
    b.ApplyMino(i1);
    std::cout << i1.x << " " << i1.y << "\n";
    std::cout << b << "\n";
    
    tetris::IMino i2(b);
    i2.Translate(-2,-19, b);
    b.ApplyMino(i2);
    std::cout << i2.x << " " << i2.y << "\n";
    std::cout << b << "\n";
    
    tetris::IMino i3(b);
    i3.Translate(3,-20, b);
    b.ApplyMino(i3);
    std::cout << i3.x << " " << i3.y << "\n";
    std::cout << b << "\n";
    
    tetris::LMino l1(b);
    l1.Translate(4,-19, b);
    b.ApplyMino(l1);
    std::cout << l1.x << " " << l1.y << "\n";
    std::cout << b << "\n";
    
    tetris::SMino s1(b);
    s1.Translate(0,-18, b);
    s1.Rotate(tetris::Rotation::Clockwise, b);
    tetris::RotationContext r = s1.Rotate(tetris::Rotation::Clockwise, b);
    assert(r.kicked);
    b.ApplyMino(s1);
    std::cout << s1.x << " " << s1.y << "\n";
    std::cout << b << "\n";
}
