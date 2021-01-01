#include "core/TerminalColors.hpp"
#include "core/Tetris.hpp"
#include <utility>
#include <iostream>

using namespace tetris;

// Board

void Board::FillSquare(int x, int y, Tile t) {
    assert(x >= 0 && x < width);
    assert(y >= 0 && y < height);

    board_[x + y*width] = t;
}

Tile Board::GetSquare(int x, int y) const {
    assert(x >= 0 && x < width);
    assert(y >= 0 && y < height);

    return board_[x + y*width];
}

void Board::ApplyMino(Mino& mino) {
    for(std::vector<int> c : mino.GetFilledCoordinates()) {
        FillSquare(c[0], c[1], mino.GetTile());
    }
}

int Board::UpdateBoard() {
    std::vector<int> cleared;
    for(int i = 0; i < height; i++) {
        int cnt = 0;
        for(int j = 0; j < width; j++) {
            if(GetSquare(j, i) != Tile::Empty) {
                cnt++;
            }
        }
        if(cnt == width) {
            cleared.push_back(i);
        }
    }
    for(int i = 0; i < height; i++) {
        int shift = 0;
        bool skip = false;
        for(int x : cleared) {
            if(i > x) {
                shift++;
            }
            if(i == x) {
                skip = true;
            }
        }
        if(skip) {
            continue;
        }
        for(int j = 0; j < width; j++) {
            FillSquare(j, i-shift, GetSquare(j, i));
        }
    }
}

std::string Board::to_string() const {
    std::string s = "";
    for(int i = height-1; i >= 0; i--) {
        for(int j = 0; j < width; j++) {
            switch(GetSquare(j, i)) {
                case I:
                    s += CYAN;
                    s += 'I';
                    break;
                case L:
                    // 8 bit color can't make orange
                    s += WHITE;
                    s += 'L';
                    break;
                case J:
                    s += BLUE;
                    s += 'J';
                    break;
                case O:
                    s += YELLOW;
                    s += 'O';
                    break;
                case S:
                    s += GREEN;
                    s += 'S';
                    break;
                case Z:
                    s += RED;
                    s += 'Z';
                    break;
                case T:
                    s += MAGENTA;
                    s += 'T';
                    break;
                case Empty:
                    s += RESET;
                    s += '-';
                    break;
                case X:
                    s += WHITE;
                    s += 'X';
                    break;
            }
        }
        s += '\n';
    }
    s += RESET;
    return s;
}

std::ostream &tetris::operator<<(std::ostream &os, const Board &b) {
    return os << b.to_string();
}

void Mino::InternalTranslate(int x, int y) {
    this->x += x;
    this->y += y;
}

void Mino::InternalRotate(Rotation r) {
    // Transpose the matrix
    for(int i = 0; i < bb_h; i++) {
        for(int j = 0; j < i; j++) {
            Tile tmp = bounding_box_[i][j];
            bounding_box_[i][j] = bounding_box_[j][i];
            bounding_box_[j][i] = tmp;
        }
    }
    if(r == Rotation::CounterClockwise) {
        orientation_ = (orientation_ - 1 + 4) % 4;
        // Reflect over x-axis
        for(int i = 0; i < bb_h/2; i++) {
            for(int j = 0; j < bb_w; j++) {
                Tile tmp = bounding_box_[i][j];
                bounding_box_[i][j] = bounding_box_[bb_h - i - 1][j];
                bounding_box_[bb_h - i - 1][j] = tmp;
            }
        }
    }
    if(r == Rotation::Clockwise) {
        orientation_ = (orientation_ + 1) % 4;
        // Reflect over y-axis
        for(int i = 0; i < bb_h; i++) {
            for(int j = 0; j < bb_w/2; j++) {
                Tile tmp = bounding_box_[i][j];
                bounding_box_[i][j] = bounding_box_[i][bb_w - j - 1];
                bounding_box_[i][bb_w - j - 1] = tmp;
            }
        }
    }
}

bool Mino::Translate(int x, int y, Board& board) {
    InternalTranslate(x, y);
    if(Colliding(board)) {
        InternalTranslate(-x, -y);
        return false;
    }
    return true;
}

RotationContext Mino::Rotate(Rotation r, Board& board) {
    Rotation r_inv;
    switch(r) {
        case Rotation::Clockwise:
            r_inv = Rotation::CounterClockwise;
            break;
        case Rotation::CounterClockwise:
            r_inv = Rotation::Clockwise;
            break;
    }

    int original_x = x;
    int original_y = y;
    int original_orientation = orientation_;

    InternalRotate(r);
    int new_orientation = orientation_;
    int kick_table_sz = kick_table_[std::make_pair(original_orientation, new_orientation)].size();
    int kick_table_idx = 0;

    while(kick_table_idx < kick_table_sz) {
        // remove kick from previous iteration
        x = original_x;
        y = original_y;
       
        // apply kick
        x += std::get<0>(kick_table_[std::make_pair(original_orientation, new_orientation)][kick_table_idx]);
        y += std::get<1>(kick_table_[std::make_pair(original_orientation, new_orientation)][kick_table_idx]);

        if(!Colliding(board)) {
            std::cout << kick_table_idx << "\n";
            break;
        }

        kick_table_idx++;
    }

    // None of the kicks are satisfied
    if(kick_table_idx >= kick_table_sz) {
        // Undo rotations and translations
        x = original_x;
        y = original_y;
        InternalRotate(r_inv);
        return {false, false};
    }
    return {true, kick_table_idx > 0};
}

bool Mino::Colliding(Board& board) {
    for(int i = 0; i < bb_h; i++) {
        for(int j = 0; j < bb_w; j++) {
            if(j+x < 0 || j+x >= board.width) {
                // Piece of mino is outside the board
                if(bounding_box_[i][j] != Tile::Empty) {
                    return true;
                }
                // Otherwise it's just a portion of the bounding box outside the board
                continue;    
            }
            if(y-i < 0 || y-i >= board.height) {
                // Piece of mino is outside the board
                if(bounding_box_[i][j] != Tile::Empty) {
                    return true;
                }
                // Otherwise it's just a portion of the bounding box outside the board
                continue;    
            }
            // Inside the board, just check if tiles overlap
            if(bounding_box_[i][j] != Tile::Empty && board.GetSquare(j+x, y-i) != Tile::Empty) {
                return true;
            }
        }
    }
    return false;
}

std::string Mino::to_string() const {
    std::string s = "";
    for(int i = 0; i < bb_h; i++) {
        for(int j = 0; j < bb_w; j++) {
            switch(bounding_box_[i][j]) {
                case I:
                    s += CYAN;
                    s += 'I';
                    break;
                case L:
                    s += WHITE;
                    s += 'L';
                    break;
                case J:
                    s += BLUE;
                    s += 'J';
                    break;
                case O:
                    s += YELLOW;
                    s += 'O';
                    break;
                case S:
                    s += GREEN;
                    s += 'S';
                    break;
                case Z:
                    s += RED;
                    s += 'Z';
                    break;
                case T:
                    s += MAGENTA;
                    s += 'T';
                    break;
                case Empty:
                    s += RESET;
                    s += ' ';
                    break;
                case X:
                    s += WHITE;
                    s += 'X';
                    break;
            }
        }
        s += '\n';
    }
    s += RESET;
    return s;
}

bool TMino::IsTSpin() {
    //TODO: Tspin detection
}

bool TMino::IsMiniTSpin() {
    //TODO: Mini Tspin detection
}

std::ostream &tetris::operator<<(std::ostream &os, const Mino &a) {
    return os << a.to_string();
}

bool Mino::operator==(const Mino& b) const {
    return GetFilledCoordinates() == b.GetFilledCoordinates();
}

std::vector<std::vector<int>> Mino::GetFilledCoordinates() const {
    std::vector<std::vector<int>> coords;
    for(int i = 0; i < bb_h; i++) {
        for(int j = 0; j < bb_w; j++) {
            if(bounding_box_[i][j] != tetris::Tile::Empty) {
               coords.push_back({x+j, y-i}); 
            }
        }
    }
    return coords;
}
