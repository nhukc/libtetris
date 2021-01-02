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
    PointVec points = mino.GetFilledCoordinates();
    for(int i = 0; i < points.Size(); i++) {
        FillSquare(points.Get(i,0), points.Get(i,1), mino.GetTile());
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
    PointVec points = GetFilledCoordinates();
    for(int i = 0; i < points.Size(); i++) {
        int x = points.Get(i,0);
        int y = points.Get(i,1);
        if(x < 0 || x >= board.width) {
            return true;
        }
        if(y < 0 || y >= board.height) {
            return true;
        }
        if(board.GetSquare(x, y) != Tile::Empty) {
            return true;
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

template<int N>
constexpr std::array<std::array<int, 2>, 4> GetFilledCoordinates_(std::array<std::array<Tile,N>,N> bounding_box) {
    std::array<std::array<int, 2>, 4> coords;
    int c = 0;
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {
            if(bounding_box[i][j] != tetris::Tile::Empty) {
                coords[c][0] = j;
                coords[c][1] = i;
                c++;     
            }
        }
    }
    return coords;
}

// Computes rotations at compile-time
template<int N>
constexpr std::array<std::array<Tile,N>, N> rotate(std::array<std::array<Tile,N>, N> array) {
    std::array<std::array<Tile, N>, N> result;
    // Transpose the matrix
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {
            result[i][j] = array[j][i];
        }
    }
    // Reflect over y-axis
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N/2; j++) {
            Tile tmp = result[i][j];
            result[i][j] = result[i][N - j - 1];
            result[i][N - j - 1] = tmp;
        }
    }
    return result;
}

inline PointVec Mino::GetFilledCoordinatesSlow() const {
    PointVec coords;
    for(int i = 0; i < bb_h; i++) {
        for(int j = 0; j < bb_w; j++) {
            if(bounding_box_[i][j] != tetris::Tile::Empty) {
               coords.Push(x+j, y-i); 
            }
        }
    }
    return coords;
}

// This function is very performance sensitive
// We do some magic with constexpr and templating to frontload computations to compile-time
inline PointVec Mino::GetFilledCoordinates() const {
    switch(tile_type) { 
        case Tile::O: {
            constexpr std::array<std::array<Tile,2>,2> arr({
                    std::array<Tile,2>({Tile::O, Tile::O}), 
                    std::array<Tile,2>({Tile::O, Tile::O}), 
            });
            // Without intermediate constexpr variables, these will be evaluated at runtime
            constexpr std::array<std::array<int, 2>, 4> coords0 = GetFilledCoordinates_<2>(arr);
            constexpr std::array<std::array<int, 2>, 4> coords1 = GetFilledCoordinates_<2>(rotate<2>(arr));
            constexpr std::array<std::array<int, 2>, 4> coords2 = GetFilledCoordinates_<2>(rotate<2>(rotate<2>(arr)));
            constexpr std::array<std::array<int, 2>, 4> coords3 = GetFilledCoordinates_<2>(rotate<2>(rotate<2>(rotate<2>(arr))));
            PointVec coords;
            switch(orientation_) {
                case 0:
                    for(int i = 0; i < 4; i++) {
                        coords.Push(x+coords0[i][0], y-coords0[i][1]);
                    }
                    break;
                case 1:
                    for(int i = 0; i < 4; i++) {
                        coords.Push(x+coords1[i][0], y-coords1[i][1]);
                    }
                    break;
                case 2:
                    for(int i = 0; i < 4; i++) {
                        coords.Push(x+coords2[i][0], y-coords2[i][1]);
                    }
                    break;
                case 3:
                    for(int i = 0; i < 4; i++) {
                        coords.Push(x+coords3[i][0], y-coords3[i][1]);
                    }
                    break;
            }
            return coords;
        }
        case Tile::L: {
            std::array<std::array<int, 2>, 4> coords_;
            constexpr std::array<std::array<Tile,3>,3> arr({
                    std::array<Tile,3>({Tile::Empty, Tile::Empty, Tile::L}), 
                    std::array<Tile,3>({Tile::L,     Tile::L,     Tile::L}), 
                    std::array<Tile,3>({Tile::Empty, Tile::Empty, Tile::Empty}), 
            });
            // Without intermediate constexpr variables, these will be evaluated at runtime
            constexpr std::array<std::array<int, 2>, 4> coords0 = GetFilledCoordinates_<3>(arr);
            constexpr std::array<std::array<int, 2>, 4> coords1 = GetFilledCoordinates_<3>(rotate<3>(arr));
            constexpr std::array<std::array<int, 2>, 4> coords2 = GetFilledCoordinates_<3>(rotate<3>(rotate<3>(arr)));
            constexpr std::array<std::array<int, 2>, 4> coords3 = GetFilledCoordinates_<3>(rotate<3>(rotate<3>(rotate<3>(arr))));
            PointVec coords;
            switch(orientation_) {
                case 0:
                    for(int i = 0; i < 4; i++) {
                        coords.Push(x+coords0[i][0], y-coords0[i][1]);
                    }
                    break;
                case 1:
                    for(int i = 0; i < 4; i++) {
                        coords.Push(x+coords1[i][0], y-coords1[i][1]);
                    }
                    break;
                case 2:
                    for(int i = 0; i < 4; i++) {
                        coords.Push(x+coords2[i][0], y-coords2[i][1]);
                    }
                    break;
                case 3:
                    for(int i = 0; i < 4; i++) {
                        coords.Push(x+coords3[i][0], y-coords3[i][1]);
                    }
                    break;
            }
            return coords;
        }
        case Tile::J: {
            std::array<std::array<int, 2>, 4> coords_;
            constexpr std::array<std::array<Tile,3>,3> arr({
                    std::array<Tile,3>({Tile::J,     Tile::Empty, Tile::Empty}), 
                    std::array<Tile,3>({Tile::J,     Tile::J,     Tile::J}), 
                    std::array<Tile,3>({Tile::Empty, Tile::Empty, Tile::Empty}), 
            });
            // Without intermediate constexpr variables, these will be evaluated at runtime
            constexpr std::array<std::array<int, 2>, 4> coords0 = GetFilledCoordinates_<3>(arr);
            constexpr std::array<std::array<int, 2>, 4> coords1 = GetFilledCoordinates_<3>(rotate<3>(arr));
            constexpr std::array<std::array<int, 2>, 4> coords2 = GetFilledCoordinates_<3>(rotate<3>(rotate<3>(arr)));
            constexpr std::array<std::array<int, 2>, 4> coords3 = GetFilledCoordinates_<3>(rotate<3>(rotate<3>(rotate<3>(arr))));
            PointVec coords;
            switch(orientation_) {
                case 0:
                    for(int i = 0; i < 4; i++) {
                        coords.Push(x+coords0[i][0], y-coords0[i][1]);
                    }
                    break;
                case 1:
                    for(int i = 0; i < 4; i++) {
                        coords.Push(x+coords1[i][0], y-coords1[i][1]);
                    }
                    break;
                case 2:
                    for(int i = 0; i < 4; i++) {
                        coords.Push(x+coords2[i][0], y-coords2[i][1]);
                    }
                    break;
                case 3:
                    for(int i = 0; i < 4; i++) {
                        coords.Push(x+coords3[i][0], y-coords3[i][1]);
                    }
                    break;
            }
            return coords;
        }
        case Tile::S: {
            std::array<std::array<int, 2>, 4> coords_;
            constexpr std::array<std::array<Tile,3>,3> arr({
                    std::array<Tile,3>({Tile::Empty, Tile::S,     Tile::S}), 
                    std::array<Tile,3>({Tile::S,     Tile::S,     Tile::Empty}), 
                    std::array<Tile,3>({Tile::Empty, Tile::Empty, Tile::Empty}), 
            });
            // Without intermediate constexpr variables, these will be evaluated at runtime
            constexpr std::array<std::array<int, 2>, 4> coords0 = GetFilledCoordinates_<3>(arr);
            constexpr std::array<std::array<int, 2>, 4> coords1 = GetFilledCoordinates_<3>(rotate<3>(arr));
            constexpr std::array<std::array<int, 2>, 4> coords2 = GetFilledCoordinates_<3>(rotate<3>(rotate<3>(arr)));
            constexpr std::array<std::array<int, 2>, 4> coords3 = GetFilledCoordinates_<3>(rotate<3>(rotate<3>(rotate<3>(arr))));
            PointVec coords;
            switch(orientation_) {
                case 0:
                    for(int i = 0; i < 4; i++) {
                        coords.Push(x+coords0[i][0], y-coords0[i][1]);
                    }
                    break;
                case 1:
                    for(int i = 0; i < 4; i++) {
                        coords.Push(x+coords1[i][0], y-coords1[i][1]);
                    }
                    break;
                case 2:
                    for(int i = 0; i < 4; i++) {
                        coords.Push(x+coords2[i][0], y-coords2[i][1]);
                    }
                    break;
                case 3:
                    for(int i = 0; i < 4; i++) {
                        coords.Push(x+coords3[i][0], y-coords3[i][1]);
                    }
                    break;
            }
            return coords;
        }
        case Tile::Z: {
            std::array<std::array<int, 2>, 4> coords_;
            constexpr std::array<std::array<Tile,3>,3> arr({
                    std::array<Tile,3>({Tile::Z,     Tile::Z,     Tile::Empty}), 
                    std::array<Tile,3>({Tile::Empty, Tile::Z,     Tile::Z}), 
                    std::array<Tile,3>({Tile::Empty, Tile::Empty, Tile::Empty}), 
            });
            // Without intermediate constexpr variables, these will be evaluated at runtime
            constexpr std::array<std::array<int, 2>, 4> coords0 = GetFilledCoordinates_<3>(arr);
            constexpr std::array<std::array<int, 2>, 4> coords1 = GetFilledCoordinates_<3>(rotate<3>(arr));
            constexpr std::array<std::array<int, 2>, 4> coords2 = GetFilledCoordinates_<3>(rotate<3>(rotate<3>(arr)));
            constexpr std::array<std::array<int, 2>, 4> coords3 = GetFilledCoordinates_<3>(rotate<3>(rotate<3>(rotate<3>(arr))));
            PointVec coords;
            switch(orientation_) {
                case 0:
                    for(int i = 0; i < 4; i++) {
                        coords.Push(x+coords0[i][0], y-coords0[i][1]);
                    }
                    break;
                case 1:
                    for(int i = 0; i < 4; i++) {
                        coords.Push(x+coords1[i][0], y-coords1[i][1]);
                    }
                    break;
                case 2:
                    for(int i = 0; i < 4; i++) {
                        coords.Push(x+coords2[i][0], y-coords2[i][1]);
                    }
                    break;
                case 3:
                    for(int i = 0; i < 4; i++) {
                        coords.Push(x+coords3[i][0], y-coords3[i][1]);
                    }
                    break;
            }
            return coords;
        }
        case Tile::I: {
            std::array<std::array<int, 2>, 4> coords_;
            constexpr std::array<std::array<Tile,4>,4> arr({
                    std::array<Tile,4>({Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty}), 
                    std::array<Tile,4>({Tile::I,     Tile::I,     Tile::I,     Tile::I}), 
                    std::array<Tile,4>({Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty}), 
                    std::array<Tile,4>({Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty}), 
            });
            // Without intermediate constexpr variables, these will be evaluated at runtime
            constexpr std::array<std::array<int, 2>, 4> coords0 = GetFilledCoordinates_<4>(arr);
            constexpr std::array<std::array<int, 2>, 4> coords1 = GetFilledCoordinates_<4>(rotate<4>(arr));
            constexpr std::array<std::array<int, 2>, 4> coords2 = GetFilledCoordinates_<4>(rotate<4>(rotate<4>(arr)));
            constexpr std::array<std::array<int, 2>, 4> coords3 = GetFilledCoordinates_<4>(rotate<4>(rotate<4>(rotate<4>(arr))));
            PointVec coords;
            switch(orientation_) {
                case 0:
                    for(int i = 0; i < 4; i++) {
                        coords.Push(x+coords0[i][0], y-coords0[i][1]);
                    }
                    break;
                case 1:
                    for(int i = 0; i < 4; i++) {
                        coords.Push(x+coords1[i][0], y-coords1[i][1]);
                    }
                    break;
                case 2:
                    for(int i = 0; i < 4; i++) {
                        coords.Push(x+coords2[i][0], y-coords2[i][1]);
                    }
                    break;
                case 3:
                    for(int i = 0; i < 4; i++) {
                        coords.Push(x+coords3[i][0], y-coords3[i][1]);
                    }
                    break;
            }
            return coords;
        }
        case Tile::T: {
            std::array<std::array<int, 2>, 4> coords_;
            constexpr std::array<std::array<Tile,3>,3> arr({
                    std::array<Tile,3>({Tile::Empty, Tile::T,     Tile::Empty}), 
                    std::array<Tile,3>({Tile::T,     Tile::T,     Tile::T}), 
                    std::array<Tile,3>({Tile::Empty, Tile::Empty, Tile::Empty}), 
            });
            // Without intermediate constexpr variables, these will be evaluated at runtime
            constexpr std::array<std::array<int, 2>, 4> coords0 = GetFilledCoordinates_<3>(arr);
            constexpr std::array<std::array<int, 2>, 4> coords1 = GetFilledCoordinates_<3>(rotate<3>(arr));
            constexpr std::array<std::array<int, 2>, 4> coords2 = GetFilledCoordinates_<3>(rotate<3>(rotate<3>(arr)));
            constexpr std::array<std::array<int, 2>, 4> coords3 = GetFilledCoordinates_<3>(rotate<3>(rotate<3>(rotate<3>(arr))));
            PointVec coords;
            switch(orientation_) {
                case 0:
                    for(int i = 0; i < 4; i++) {
                        coords.Push(x+coords0[i][0], y-coords0[i][1]);
                    }
                    break;
                case 1:
                    for(int i = 0; i < 4; i++) {
                        coords.Push(x+coords1[i][0], y-coords1[i][1]);
                    }
                    break;
                case 2:
                    for(int i = 0; i < 4; i++) {
                        coords.Push(x+coords2[i][0], y-coords2[i][1]);
                    }
                    break;
                case 3:
                    for(int i = 0; i < 4; i++) {
                        coords.Push(x+coords3[i][0], y-coords3[i][1]);
                    }
                    break;
            }
            return coords;
        }
    }
    return GetFilledCoordinatesSlow();
}
