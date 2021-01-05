#ifndef MOVEGEN_HPP
#define MOVEGEN_HPP
#include "core/Randomizers.hpp"
#include "core/Tetris.hpp"
#include "third_party/uint256_t.h"

#include <climits>
#include <iostream>
#include <memory>

// A tetris board can be represented as a 20x10 bits
typedef uint256_t bit_board;

// More efficient than using minos
struct move_info {
    int x, y;
    int orientation;
    tetris::Tile piece_type;
};

bit_board ToBitBoard(tetris::Board board);
tetris::Mino* FromMoveInfo(move_info move);
bit_board MoveToBitBoard(move_info move);

// Given a board and a mino, generates all possible new mino positions
std::vector<move_info> FindPossiblePositions(bit_board board, tetris::Tile mino);

// Given a start board and target board, finds action sequences that transform start to target
std::vector<std::vector<std::unique_ptr<tetris::Mino>>> FindTargetBoard(tetris::Board start, tetris::Board target, tetris::Randomizer& randomizer);

#endif
