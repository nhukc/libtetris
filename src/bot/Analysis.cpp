#include "bot/Analysis.hpp"
#include <stack>

void AnalysisContext::IterativeDeepen(int depth, bit_board board, std::vector<tetris::Tile> minos) {
    mino_sequence = minos;
    for(int d = 1; d <= depth; d++) {
        DFS(d, board, 0, -1, minos[0], 0, false, 0, false);
        transposition_matrix.clear();
    }
    std::reverse(best_moves.begin(), best_moves.end());
}

int update_board(bit_board& board, move_info& move, bool& back_to_back, int& combo) {
    int cnt = 0;
    bit_board row = 0b1111111111;
    bit_board below_row_mask = 0;
    const int width = 10;
    const int height = 20;

    bool tspin = false;
    bool tspin_mini = false;

    // T-spin detection
    if(move.piece_type == tetris::Tile::T) {
        // If 2 of these are set and 1 mini bit is set, this is a tspin
        bit_board tspin_mask;
        // If the piece is against the wall, we need to prevent bounds errors
        bool tspin_mask_oob = false;
        // If 2 of these are set  and 1 tspin bit is set, this is a mini tspin
        bit_board tspin_mini_mask;
        bool tspin_mini_mask_oob = false;

        switch(move.orientation) {
            // See tetris guideline for where these numbers come from
            case 0:
                tspin_mask = 0b000 | (0b000 << 10) | (0b101 << 20);
                tspin_mini_mask = 0b101 | (0b000 << 10) | (0b000 << 20);
                if(move.y == 1) {
                    tspin_mini_mask_oob = true;
                }
                break;
            case 1:
                tspin_mask = 0b100 | (0b000 << 10) | (0b100 << 20);
                tspin_mini_mask = 0b001 | (0b000 << 10) | (0b001 << 20);
                if(move.x == -1) {
                    tspin_mini_mask_oob = true;
                }
                break;
            case 2:
                tspin_mask = 0b101 | (0b000 << 10) | (0b000 << 20);
                tspin_mini_mask = 0b000 | (0b000 << 10) | (0b101 << 20);
                break;
            case 3:
                tspin_mask = 0b001 | (0b000 << 10) | (0b001 << 20);
                tspin_mini_mask = 0b100 | (0b000 << 10) | (0b100 << 20);
                if(move.x >= width-2) {
                    tspin_mini_mask_oob = true;
                }
                break;
        }

        // Move the board mask over the T piece
        // Y coordinate -2 since pieces are positioned by top left corner
        tspin_mask <<= (move.x + 10*(move.y-2));
        tspin_mini_mask <<= (move.x + 10*(move.y-2));

        // If we're at the edge of the board there are no blocks to check against so we set true
        // If we're not, we check against the mask
        bool tspin_two_bits_set = tspin_mask_oob ? true : (tspin_mask & board) == tspin_mask;
        bool tspin_one_bit_set = tspin_mask_oob ? true : (tspin_mask & board) != 0;
        bool tspin_mini_two_bits_set = tspin_mini_mask_oob ? true : (tspin_mini_mask & board) == tspin_mini_mask;
        bool tspin_mini_one_bit_set = tspin_mini_mask_oob ? true : (tspin_mini_mask & board) != 0;
            
        tspin = tspin_two_bits_set && tspin_mini_one_bit_set;
        tspin_mini = tspin_mini_two_bits_set && tspin_one_bit_set;
    }

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

    int sent = 0;
    if(cnt != 0) {
        switch(cnt) {
            case 1:
                sent = 0;
                if(tspin) {
                    sent = 2;
                }
                break;
            case 2:
                if(tspin) {
                    sent = 4;
                }
                else {
                    sent = 1;
                }
                break;
            case 3:
                if(tspin) {
                    sent = 6;
                }
                else {
                    sent = 2;
                }
                break;
            case 4:
                sent = 4;
                break;
        }
        
        // If we should add a back-to-back to this attack
        bool back_to_back_add = (tspin || tspin_mini || cnt == 4) && back_to_back;
        sent = sent + (back_to_back_add ? 1 : 0);

        // If the back-to-back should be set for subsequent attacks
        if(tspin || tspin_mini || cnt == 4)
            back_to_back = true;
        else
            back_to_back = false;

        // Perfect clear
        if(board == 0) {
            sent += 10;
        }
    }

    return sent;
}

bool AnalysisContext::DFS(int depth, bit_board board, int curr_idx, int swap_idx, tetris::Tile curr_piece, int attack, bool back_to_back, int combo, bool back_to_back_lost) {
    // Add some metadata to the transposition matrix entry
    // Top 56 bits are 0 anyway so we can add whatever we want here
    bit_board transposition = board 
        | ((bit_board)(attack&0xff) << 248) 
        | ((bit_board)(curr_idx&0xff) << 240) 
        | ((bit_board)(swap_idx&0xff) << 232)
        | ((bit_board)(back_to_back&0xff) << 224)
        | ((bit_board)(combo&0xff) << 216);
    if(transposition_matrix.count(transposition) != 0) {
        return false;
    }
    transposition_matrix.insert(transposition);
    if(depth == 0 || curr_idx > mino_sequence.size()) {
        int score = Evaluate(board, attack, back_to_back_lost, combo);
        if(score > best_score) {
            // TODO: Check board is actually reachable

            // Clear the best move sequence since our parent calls will update it
            std::cout << board << "\n";
            best_score = score;
            best_moves.clear();
            for(int i = 19; i >= 0; i--) {
                for(int j = 0; j < 10; j++) {
                    if(board & ((bit_board)1 << (i*10 + j))) {
                        std::cout << "X";
                    }
                    else {
                        std::cout << "-";
                    }
                }
                std::cout << "\n";
            }
            std::cout << "\n";
            return true;
        }
        return false;
    }
    std::vector<move_info> possible_moves = FindPossiblePositions(board, curr_piece);
    bool found_new_best = false;
    for(auto next_move : possible_moves) {
        bit_board next_board = board | MoveToBitBoard(next_move);
        bool new_back_to_back = back_to_back;
        int new_combo = combo;
        int new_attack = attack + update_board(next_board, next_move, new_back_to_back, new_combo);
        if(DFS(depth-1, next_board, curr_idx + 1, swap_idx, mino_sequence[curr_idx+1], new_attack, new_back_to_back, new_combo, back_to_back && !new_back_to_back)) {
            // This move found a new best, add it to the best move sequence
            best_moves.push_back(next_move);
            found_new_best = true;
        }
    }
    return found_new_best;
}

// This code is extremely performance sensitive
int AnalysisContext::Evaluate(bit_board board, int attack, bool back_to_back_lost, int combo) {
    int score = attack*10;

    // Check for overhangs/holes
    // Each is worth -2 attack
    //
    // Bitwise approach instead of looping over every bit decreases time from .73s to .2s
    // on a three depth search.
    bit_board row = 0b1111111111;
    bit_board above = row << 10;
    for(int i = 0; i < 19; i++) {
        bit_board check = ~((row & board) << 10) & (above & board);
        // We shouldn't hit this check very often
        if(check) {
            for(int j = 0; j < 10; j++) {
                if(((bit_board)1 << j) & check) {
                    score -= 20;
                }
            }
        }
    }

    // Back to back loss worth -2 attack
    // This will probably give all initial boards a negative score
    if(back_to_back_lost) {
        score -= 20;
    }

    return score;
}
