#ifndef ANALYSIS_HPP
#define ANALYSIS_HPP
#include "MoveGen.hpp"
#include <unordered_map>
#include <unordered_set>

struct quiescence_entry {
    bit_board board;
    tetris::Tile swap;
    int depth;
};

// hopefully you're running a 64-bit machine :)
struct hash {
    size_t operator()(const bit_board& b) const {
        return (b >> 192) ^ (b >> 128) ^ (b >> 64) ^ b;
    }
};


struct AnalysisContext {
    std::unordered_set<bit_board, hash> transposition_matrix;
    std::vector<quiescence_entry> quiescence_list;
    std::vector<tetris::Tile> mino_sequence;
    std::vector<move_info> best_moves;
    int best_score = INT_MIN;

    // Use iterative deepening to keep memory usage low
    // Iterative deepening calls DFS with a limited depth
    void IterativeDeepen(int depth, bit_board board, std::vector<tetris::Tile> minos);

    // Helper DFS function
    bool DFS(int depth, bit_board board, int curr_idx, int swap_idx, tetris::Tile curr_piece, int attack, bool back_to_back, int combo, bool back_to_back_lost);
    
    // Heuristic to determine the "score" of a board
    int Evaluate(bit_board board, int attack, bool back_to_back, int combo);
};

#endif
