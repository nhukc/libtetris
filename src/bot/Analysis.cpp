#include "bot/Analysis.hpp"
#include <stack>

void AnalysisContext::IterativeDeepen(int depth, bit_board board, std::vector<tetris::Tile> minos) {
    mino_sequence = minos;
    for(int d = 1; d <= depth; d++) {
        DFS(d, board, 0, -1, minos[0], 0);
        transposition_matrix.clear();
    }
}

int update_board(bit_board& board, move_info& move) {
    int cnt = 0;
    bit_board row = 0b1111111111;
    bit_board below_row_mask = 0;
    for(int i = 0; i < 20; i++) {
        if((row & board) == row) {
            // Everything below current row + Everything above current row shifted by one row
            board = (below_row_mask & board) | (((~(row | below_row_mask) & board)) >> 10);
            cnt++;
            // If we reduced a row, we need to decrement to prevent skipping a row
            i -= 1;
            continue;
        }
        below_row_mask |= row;
        row <<= 10;
    }
    return cnt;
}

bool AnalysisContext::DFS(int depth, bit_board board, int curr_idx, int swap_idx, tetris::Tile curr_piece, int attack) {
    // Add some metadata to the transposition matrix entry
    // Top 56 bits are 0 anyway
    bit_board transposition = board | ((bit_board)(attack&0xff) << 248) | ((bit_board)(curr_idx&0xff) << 240) | ((bit_board)(swap_idx&0xff) << 232);
    if(transposition_matrix.count(transposition) != 0) {
        return false;
    }
    transposition_matrix.insert(transposition);
    if(depth == 0 || curr_idx > mino_sequence.size()) {
        int score = Evaluate(board, attack);
        if(score > best_score) {
            // TODO: Check board is actually reachable

            // Clear the best move sequence since our parent calls will update it
            std::cout << board << "\n";
            best_score = score;
            best_moves.clear();
            return true;
        }
        return false;
    }
    std::vector<move_info> possible_moves = FindPossiblePositions(board, curr_piece);
    bool found_new_best = false;
    for(auto next_move : possible_moves) {
        bit_board next_board = board | MoveToBitBoard(next_move);
        int new_attack = attack + update_board(next_board, next_move);
        if(DFS(depth-1, next_board, curr_idx + 1, swap_idx, mino_sequence[curr_idx+1], new_attack)) {
            // This move found a new best, add it to the best move sequence
            best_moves.push_back(next_move);
            found_new_best = true;
        }
    }
    return found_new_best;
}

int AnalysisContext::Evaluate(bit_board b, int attack) {
    return attack;
}
