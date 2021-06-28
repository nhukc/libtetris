#ifndef FUMEN_HPP
#define FUMEN_HPP

#include <string>
#include "../core/Tetris.hpp"

class Fumen {
    public:
    std::vector<tetris::Board> pages;
};

std::string encode(Fumen f);
Fumen decode(std::string);

int ToInt(tetris::Tile x);
tetris::Tile ToTile(int x);

#endif
