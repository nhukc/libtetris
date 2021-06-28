#include "bot/MoveGen.hpp"
#include <iostream>

// Empty bitboard with a piece in the bottom left corner
bit_board piece_boards[7][4] = {
    // Rows are reflected from what you'd see in a picture
    // O pieces
    {
        0b11 | (0b11 << 10),
        0b11 | (0b11 << 10),
        0b11 | (0b11 << 10),
        0b11 | (0b11 << 10),
    },
    // L pieces
    {
        0b000 | (0b111 << 10) | (0b100 << 20),
        0b110 | (0b010 << 10) | (0b010 << 20),
        0b001 | (0b111 << 10) | (0b000 << 20),
        0b010 | (0b010 << 10) | (0b011 << 20),
    },
    // J pieces
    {
        0b000 | (0b111 << 10) | (0b001 << 20),
        0b010 | (0b010 << 10) | (0b110 << 20),
        0b100 | (0b111 << 10) | (0b000 << 20),
        0b011 | (0b010 << 10) | (0b010 << 20),
    },
    // S pieces
    {
        0b000 | (0b011 << 10) | (0b110 << 20),
        0b100 | (0b110 << 10) | (0b010 << 20),
        0b011 | (0b110 << 10) | (0b000 << 20),
        0b010 | (0b011 << 10) | (0b001 << 20),
    },
    // Z pieces
    {
        0b000 | (0b110 << 10) | (0b011 << 20),
        0b010 | (0b110 << 10) | (0b100 << 20),
        0b110 | (0b011 << 10) | (0b000 << 20),
        0b001 | (0b011 << 10) | (0b010 << 20),
    },
    // T pieces
    {
        0b000 | (0b111 << 10) | (0b010 << 20),
        0b010 | (0b110 << 10) | (0b010 << 20),
        0b010 | (0b111 << 10) | (0b000 << 20),
        0b010 | (0b011 << 10) | (0b010 << 20),
    },
    // I pieces
    {
        // Cast to avoid 32bit precision issues
        0b0000 | (0b0000 << 10) | (0b1111 << 20) | ((bit_board)0b0000 << 30),
        0b0100 | (0b0100 << 10) | (0b0100 << 20) | ((bit_board)0b0100 << 30),
        0b0000 | (0b1111 << 10) | (0b0000 << 20) | ((bit_board)0b0000 << 30),
        0b0010 | (0b0010 << 10) | (0b0010 << 20) | ((bit_board)0b0010 << 30),
    },
};

struct piece_constants {
    // x_min offset from 0
    int x_min;
    // x_max offset from width
    int x_max;
    // y_min offset from 0
    int y_min;
    // y_max offset from height
    int y_max;
    // bounding box sizes
    int bb_h;
    int bb_w;
};

piece_constants piece_constants[7][4] {
    // O
    {
        {0, -2, 1, -1, 2, 2},
        // These 3 are unused
        {0, -2, 1, -1, 2, 2},
        {0, -2, 1, -1, 2, 2},
        {0, -2, 1, -1, 2, 2},
    },
    // L
    {
        {0, -3, 1, -1, 3, 3},
        {-1, -3, 2, -1, 3, 3},
        {0, -3, 2, 0, 3, 3},
        {0, -2, 2, -1, 3, 3},
    },
    // J
    {
        {0, -3, 1, -1, 3, 3},
        {-1, -3, 2, -1, 3, 3},
        {0, -3, 2, 0, 3, 3},
        {0, -2, 2, -1, 3, 3},
    },
    // S
    {
        {0, -3, 1, -1, 3, 3},
        {-1, -3, 2, -1, 3, 3},
        // These 2 are unused
        {0, -3, 1, -1, 3, 3},
        {-1, -3, 2, -1, 3, 3},
    },
    // Z
    {
        {0, -3, 1, -1, 3, 3},
        {-1, -3, 2, -1, 3, 3},
        // These 2 are unused
        {0, -3, 1, -1, 3, 3},
        {-1, -3, 2, -1, 3, 3},
    },
    // T
    {
        {0, -3, 1, -1, 3, 3},
        {-1, -3, 2, -1, 3, 3},
        {0, -3, 2, -1, 3, 3},
        {0, -2, 2, -1, 3, 3},
    },
    // I
    {
        {0, -4, 1, 0, 4, 4},
        {-2, -3, 3, -1, 4, 4},
        // These 2 are unused
        {0, -4, 1, 0, 4, 4},
        {-2, -3, 3, -1, 4, 4},
    },
};


bit_board ToBitBoard(tetris::Board board) {
    bit_board b = 0;
    for(int i = 0; i < board.width; i++) {
        for(int j = 0; j < board.height; j++) {
            if(board.GetSquare(i,j) != tetris::Tile::Empty)
                b = b | ((bit_board)1 << (i + j*board.width));
        }
    }
    return b;
}

tetris::Mino* FromMoveInfo(move_info move) {
    switch(move.piece_type) {
        case tetris::Tile::O:
            return new tetris::OMino(move.x, move.y, move.orientation);
        case tetris::Tile::I:
            return new tetris::IMino(move.x, move.y, move.orientation);
        case tetris::Tile::L:
            return new tetris::LMino(move.x, move.y, move.orientation);
        case tetris::Tile::J:
            return new tetris::JMino(move.x, move.y, move.orientation);
        case tetris::Tile::T:
            return new tetris::TMino(move.x, move.y, move.orientation);
        case tetris::Tile::Z:
            return new tetris::ZMino(move.x, move.y, move.orientation);
        case tetris::Tile::S:
            return new tetris::SMino(move.x, move.y, move.orientation);
    }
    assert("We shouldn't get here" == 0);
}

inline bit_board MoveToBitBoard(move_info move) {
    int idx = 0;
    switch(move.piece_type) {
        case tetris::Tile::O:
            idx = 0;
            break;
        case tetris::Tile::L:
            idx = 1;
            break;
        case tetris::Tile::J:
            idx = 2;
            break;
        case tetris::Tile::S:
            idx = 3;
            break;
        case tetris::Tile::Z:
            idx = 4;
            break;
        case tetris::Tile::T:
            idx = 5;
            break;
        case tetris::Tile::I:
            idx = 6;
            break;
    }
    const int o = move.orientation;
    // We need to make sure pieces are fully within the board
    // Bitboards can cause a "wrapping" effect with pieces that are partially outside the board
    int x_min = piece_constants[idx][o].x_min;
    int x_max = piece_constants[idx][o].x_max;
    int y_min = piece_constants[idx][o].y_min;
    int y_max = piece_constants[idx][o].y_max;
    int bb_h = piece_constants[idx][o].bb_h;
    int bb_w = piece_constants[idx][o].bb_w;
                
    bit_board b = piece_boards[idx][o];
    // Translate piece to x,y
    // Y coordinate is modified from top-left to bottom-left
    int x_shift = move.x;
    int y_shift = move.y - (bb_h - 1);
    const int board_width = 10;
    int shift = x_shift + y_shift*board_width;
    if(shift < 0) {
        b = b >> -shift;
    } else {
        b = b << shift;
    }
    return b;
}

std::vector<move_info> FindPossiblePositions(bit_board board, tetris::Tile mino_type) {
    std::vector<move_info> moves;
    int orientations = 4;
    switch(mino_type) {
        // Only need to consider one orietation for O pieces
        case tetris::Tile::O:
            orientations = 1;
            break;
        // Only need to consider two orientations for S/Z/I pieces
        case tetris::Tile::S:
        case tetris::Tile::Z:
        case tetris::Tile::I:
            orientations = 2;
            break;
    }
    // Find the highest filled tile in each column
    std::vector<int> column_heights;
    int width = 10;
    int height = 20;
    for(int i = 0; i < width; i++) {
        bool found = false;
        for(int j = height-1; j >= 0; j--) {
            if(board>>(i + j*width) & 0b1 == 1) {
                column_heights.push_back(j);
                found = true;
                break;
            }
        }
        if(!found) {
            column_heights.push_back(-1);
        }
    }
    for(int o = 0; o < orientations; o++) {
        int idx = 0;
        switch(mino_type) {
            case tetris::Tile::O:
                idx = 0;
                break;
            case tetris::Tile::L:
                idx = 1;
                break;
            case tetris::Tile::J:
                idx = 2;
                break;
            case tetris::Tile::S:
                idx = 3;
                break;
            case tetris::Tile::Z:
                idx = 4;
                break;
            case tetris::Tile::T:
                idx = 5;
                break;
            case tetris::Tile::I:
                idx = 6;
                break;
        }
        // We need to make sure pieces are fully within the board
        // Bitboards can cause a "wrapping" effect with pieces that are partially outside the board
        int x_min = 0 + piece_constants[idx][o].x_min;
        int x_max = width + piece_constants[idx][o].x_max;
        int y_min = 0 + piece_constants[idx][o].y_min;
        int y_max = height + piece_constants[idx][o].y_max;
        int bb_h = piece_constants[idx][o].bb_h;
        int bb_w = piece_constants[idx][o].bb_w;

        for(int i = x_min; i <= x_max; i++) {
            int max_height = 0;
            for(int c = 0; c < bb_w; c++) {
                if(i + c > 0 && i + c < width)
                    max_height = std::max(column_heights[i + c], max_height);
            }
            int y_bound = std::min(y_max, y_min + max_height) + 1;
            for(int j = y_min; j <= y_bound; j++) {
                bit_board b = MoveToBitBoard({i, j, o, mino_type});
                if((board & b) != 0) {
                    continue;
                }
                // No conflict and not on the first row
                if( ((b >> width) & board) == 0 && (b & 0b1111111111) == 0 ) {
                    continue;
                }                    
                moves.push_back({i, j, o, mino_type});
            }
        }
    }
    return moves;
}
