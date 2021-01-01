#include "util/Fumen.hpp"
#include <iostream>
#include <memory>

std::string encode_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
#define BOARD_WIDTH 10
#define BOARD_HEIGHT 24

// Reads fumen int of size n at string index idx
// Fumen encodes ints individually into base64
int ReadInt(std::string str, int idx, int n) {
    int result = 0;
    int radix = 1;
    for(int i = 0; i < n; i++) {
        char c = str[idx + i];
        int x = encode_table.find(c);
        result += x*radix;
        radix *= 64;
    }
    return result;
}

// Convert the numerical representation of a tile to libtetris enum
tetris::Tile ToTile(int x) {
    assert(x >= 0 && x <= 8);
    switch(x) {
        case 0:
            return tetris::Tile::Empty;
        case 1:
            return tetris::Tile::I;
        case 2:
            return tetris::Tile::L;
        case 3:
            return tetris::Tile::O;
        case 4:
            return tetris::Tile::Z;
        case 5:
            return tetris::Tile::T;
        case 6:
            return tetris::Tile::J;
        case 7:
            return tetris::Tile::S;
        case 8:
            return tetris::Tile::X;
    }
}

// Convert fumen action to libtetris mino
// Some translation adjustments are required
// T Piece rotation seems to be backwards from all other pieces...
std::unique_ptr<tetris::Mino> ToMino(int x, int y, int type, int orientation) {
    switch(type) {
        case 1:
            if(orientation == 1) {
                x -= 1;
            }
            if(orientation == 2) {
                y += 1;
            }
            return std::make_unique<tetris::IMino>(x, y, orientation);
        case 2:
            if(orientation == 1) {
                x -= 1;
            }
            if(orientation == 3) {
                x += 1;
            }
            return std::make_unique<tetris::LMino>(x, y, orientation);
        case 3:
            x += 1;
            y -= 1;
            return std::make_unique<tetris::OMino>(x, y, orientation);
        case 4:
            if(orientation == 3) {
                x += 1;
            }
            return std::make_unique<tetris::ZMino>(x, y, orientation);
        case 5:
            if(orientation == 1) {
                orientation = 3;
            }
            else if(orientation == 3) {
                orientation = 1;
            }
            return std::make_unique<tetris::TMino>(x, y, orientation);
        case 6:
            if(orientation == 1) {
                x -= 1;
            }
            if(orientation == 3) {
                x += 1;
            }
            return std::make_unique<tetris::JMino>(x, y, orientation);
        case 7:
            if(orientation == 1) {
                x -= 1;
            }
            return std::make_unique<tetris::SMino>(x, y, orientation);
    }
}

// Decodes a fumen string
Fumen decode(std::string str) {

    assert(str.find("v115@") == 0);
    // Get rid of version string
    str.erase(0, 5);

    Fumen fumen;
    // Number of times to skip whole field decodes
    int skip_field_decode = 0;
    int idx = 0;

    // Fumen starts by describing the initial field
    int curr_page = 0;
    fumen.pages.push_back(tetris::Board(BOARD_WIDTH,BOARD_HEIGHT));

    // Decode actions until we need to update the whole field
    while(idx < str.length()) {
        // Check if we should update whole field
        if(skip_field_decode == 0) {
            int cnt = 0;
            while(cnt != 240) {
                int x = ReadInt(str, idx, 2);
                idx += 2;

                // The encoding scheme of boards is based off a difference value of the
                // previous and current boards.
                int diff = x / 240;
                int run_length = (x % 240) + 1;
                std::cout << diff << " " << run_length << "\n";

                for(int i = 0; i < run_length; i++) {
                    int value;
                    if(curr_page == 0) {
                        // This is the first page, so we don't worry about an actual diff
                        value = diff - 8;
                    }
                    else {
                        value = fumen.pages[curr_page].GetSquare(cnt % BOARD_WIDTH, (BOARD_HEIGHT - 1 - cnt / BOARD_WIDTH)) + diff - 8;

                    }
                    // The runs start in the top left corner of the board, we adjust height to account for this
                    fumen.pages[0].FillSquare(cnt % BOARD_WIDTH, (BOARD_HEIGHT - 1 - cnt / BOARD_WIDTH), ToTile(value));
                    cnt++;
                }
            }
            skip_field_decode = ReadInt(str, idx, 1) + 1;
            idx += 1;
        }
        unsigned int x = ReadInt(str, idx, 3);
        idx += 3;

        int piece_type = x % 8;
        // orientation is shifted by 2 from expected
        int piece_orientation = (x / 8 + 2) % 4;
        int piece_position = (x / 32) % 240;
        int piece_x = piece_position % BOARD_WIDTH - 1;
        int piece_y = BOARD_HEIGHT - (piece_position / BOARD_WIDTH);
        int page_flags = (x / 32) / 240;

        std::cout << piece_type << " " << piece_orientation << " " << piece_x << " " << piece_y << "\n";
    
        fumen.pages[curr_page].UpdateBoard();
        if(piece_type != 0) {
            std::unique_ptr<tetris::Mino> mino = ToMino(piece_x, piece_y, piece_type, piece_orientation);
            fumen.pages[curr_page].ApplyMino(*mino);
        }

        skip_field_decode--;
        // Call the copy constructor and current page for next iteration
        fumen.pages.push_back(fumen.pages[curr_page]);
        curr_page += 1;
    }
    // Delete the extra page
    fumen.pages.pop_back();

    return fumen;
}
