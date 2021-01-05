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

struct AnalysisContext {
    std::unordered_set<bit_board> transposition_matrix;
    std::vector<quiescence_entry> quiescence_list;
    std::vector<tetris::Tile> mino_sequence;
    std::vector<move_info> best_moves;
    int best_score = INT_MIN;

    // Use iterative deepening to keep memory usage low
    // Iterative deepening calls DFS with a limited depth
    void IterativeDeepen(int depth, bit_board board, std::vector<tetris::Tile> minos);

    // Helper DFS function
    bool DFS(int depth, bit_board board, int curr_idx, int swap_idx, tetris::Tile curr_piece, int attack);
    
    // Heuristic to determine the "score" of a board
    int Evaluate(bit_board board, int attack);
};

#endif
