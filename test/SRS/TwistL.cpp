#include "core/Tetris.hpp"
#include <assert.h>
#include <iostream>

int main() {
    tetris::Board b(10,22);
    
    tetris::IAction i1(b);
    i1.Translate(0,-20);
    i1.ApplyToBoard();
    std::cout << i1.x << " " << i1.y << "\n";
    std::cout << b << "\n";
    
    tetris::IAction i2(b);
    i2.Translate(2,-19);
    i2.ApplyToBoard();
    std::cout << i2.x << " " << i2.y << "\n";
    std::cout << b << "\n";
    
    tetris::LAction l1(b);
    l1.Rotate(tetris::Rotation::CounterClockwise);
    l1.Translate(5,-19);
    tetris::Action::RotationContext r = l1.Rotate(tetris::Rotation::Clockwise);
    assert(r.kicked);
    l1.ApplyToBoard();
    std::cout << l1.x << " " << l1.y << "\n";
    std::cout << b << "\n";    
}
