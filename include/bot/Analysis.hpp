#ifndef ANALYSIS_HPP
#define ANALYSIS_HPP
#include "MoveGen.hpp"
#include <unordered_map>
#include <unordered_set>

struct search_node {
    bit_board board;
    // Depth of node in search tree
    int depth;
    // Index into mino sequence
    int curr_idx;
    // Cumulative attack for this sequence of moves
    int attack;
    // If we currenlty have a back to back
    bool back_to_back;
    // If this move made us lose a back to back
    bool back_to_back_lost;
    int combo;
};

struct quiescence_node {
    search_node node;
    std::vector<move_info> init_state;
};

// hopefully you're running a 64-bit machine :)
/*
struct hash {
    size_t operator()(const bit_board& b) const {
        return (b >> 192) ^ (b >> 128) ^ (b >> 64) ^ b;
    }
};
*/

struct AnalysisContext {
    std::unordered_set<bit_board> transposition_matrix;
    std::vector<quiescence_node> quiescence_list;
    std::vector<tetris::Tile> mino_sequence;
    std::vector<move_info> best_moves;
    std::vector<move_info> curr_moves;
    int best_score = INT_MIN;

    // Use iterative deepening to keep memory usage low
    // Iterative deepening calls DFS with a limited depth
    void IterativeDeepen(int depth, bit_board board, std::vector<tetris::Tile> minos);

    // Helper DFS function
    bool DFS(search_node node);
    
    // Heuristic to determine the "score" of a board
    int Evaluate(const search_node& node);

    // If we should prune the current node
    bool Prune(search_node& node);
};

#endif
