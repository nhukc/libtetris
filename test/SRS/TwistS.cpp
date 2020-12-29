#include "core/Tetris.hpp"
#include <assert.h>
#include <iostream>

int main() {
    tetris::Board b(10,22);
    
    tetris::IMino i1(b);
    i1.Translate(-3,-20);
    i1.ApplyToBoard();
    std::cout << i1.x << " " << i1.y << "\n";
    std::cout << b << "\n";
    
    tetris::IMino i2(b);
    i2.Translate(-2,-19);
    i2.ApplyToBoard();
    std::cout << i2.x << " " << i2.y << "\n";
    std::cout << b << "\n";
    
    tetris::IMino i3(b);
    i3.Translate(3,-20);
    i3.ApplyToBoard();
    std::cout << i3.x << " " << i3.y << "\n";
    std::cout << b << "\n";
    
    tetris::LMino l1(b);
    l1.Translate(4,-19);
    l1.ApplyToBoard();
    std::cout << l1.x << " " << l1.y << "\n";
    std::cout << b << "\n";
    
    tetris::SMino s1(b);
    s1.Translate(0,-18);
    s1.Rotate(tetris::Rotation::Clockwise);
    tetris::RotationContext r = s1.Rotate(tetris::Rotation::Clockwise);
    assert(r.kicked);
    s1.ApplyToBoard();
    std::cout << s1.x << " " << s1.y << "\n";
    std::cout << b << "\n";
}
