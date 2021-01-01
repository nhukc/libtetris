#ifndef MOVEGEN_HPP
#define MOVEGEN_HPP
#include "../core/Randomizers.hpp"
#include "../core/Tetris.hpp"

#include <memory>

// Given a board and a mino, generates all possible new mino positions
std::vector<std::unique_ptr<tetris::Mino>> FindPossiblePositions(tetris::Board board, tetris::Mino& mino);

// Given a start board and target board, finds action sequences that transform start to target
std::vector<std::vector<std::unique_ptr<tetris::Mino>>> FindTargetBoard(tetris::Board start, tetris::Board target, tetris::Randomizer& randomizer);

#endif
