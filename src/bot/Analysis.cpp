#include "bot/Analysis.hpp"
#include <numeric>
#include <stack>

int pruned = 0;
int total = 0;
void AnalysisContext::IterativeDeepen(int depth, bit_board board, std::vector<tetris::Tile> minos) {
    mino_sequence = minos;
    for(int d = 1; d <= depth; d++) {
        if(d == depth) {
            quiescence = true;
        }
        search_node init = {board, d, 0, 0, false, false, 0};
        DFS(init);
        transposition_matrix.clear();
    }
    quiescence = false;
    std::vector<quiescence_node> quiescence_copy = std::move(quiescence_list);
    // Avoid infinite looping from quiescence search adding more quiescence entries
    quiescence_list.clear();
    std::cout << "Total: " << total << "\n";
    std::cout << "Pruned: " << pruned << "\n";
    std::cout << "Quiescence: " << quiescence_copy.size() << "\n";
    int x = 0;
    for(quiescence_node& q_node : quiescence_copy) {
        curr_moves = q_node.init_state;
        search_node node = q_node.node;
        node.depth = 2;
        DFS(node);
        x++;
    }
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

bool is_tspin_shape(const bit_board& board, int x_start, int x_end, int y_start, int y_end, bool require_clearable) {
    // Center of t-piece
    // If this isn't empty, don't bother doing more work
    const bit_board center = 0b000 | (0b010 << 10) | (0b000 << 20);

    // T shapes
    const bit_board t0 = 0b000 | (0b111 << 10) | (0b010 << 20);
    const bit_board t1 = 0b010 | (0b110 << 10) | (0b010 << 20);
    const bit_board t2 = 0b010 | (0b111 << 10) | (0b000 << 20);
    const bit_board t3 = 0b010 | (0b011 << 10) | (0b010 << 20);

    // T-spin corner possibilities
    const bit_board corner0 = 0b001 | (0b000 << 10) | (0b101 << 20);
    const bit_board corner1 = 0b100 | (0b000 << 10) | (0b101 << 20);
    const bit_board corner2 = 0b101 | (0b000 << 10) | (0b001 << 20);
    const bit_board corner3 = 0b101 | (0b000 << 10) | (0b100 << 20);

    const bit_board row = 0b1111111111;

    // TODO: Check for the shape of a tpiece for quiescence search
    bit_board tspin_checker = 0;
    const int width = 10;
    const int height = 20;
    for(int i = std::max(0, x_start); i < std::min(x_end, width-3); i++) {
        for(int j = std::max(2, y_start); j < std::min(y_end, height-1); j++) {
            if(((center << (i + j*10) & board)) == 0) {
                bool hole0 = ((t0 << (i + j*10)) & board) == 0;
                bool hole1 = ((t1 << (i + j*10)) & board) == 0;
                bool hole2 = ((t2 << (i + j*10)) & board) == 0;
                bool hole3 = ((t3 << (i + j*10)) & board) == 0;
                bool filled_corner0 = ((corner0 << (i + j*10)) & board) == corner0 << (i + j*10);
                bool filled_corner1 = ((corner1 << (i + j*10)) & board) == corner1 << (i + j*10);
                bool filled_corner2 = ((corner2 << (i + j*10)) & board) == corner2 << (i + j*10);
                bool filled_corner3 = ((corner3 << (i + j*10)) & board) == corner3 << (i + j*10);
                if((hole0 || hole1 || hole2 || hole3) && (filled_corner0 || filled_corner1 || filled_corner2 || filled_corner3)) {
                    if(require_clearable) {
                        bit_board first_row = board & (row << (j)*10); 
                        bit_board second_row = board & (row << (j+1)*10); 
                        int cnt = 0;
                        for(int k = 0; k < width; k++) {
                            if(first_row & ((bit_board)1 << ((j)*10 + k))) {
                                cnt++;
                            }
                        }
                        if(cnt <= 5) {
                            continue;
                        }
                        std::cout << "Count1: " << cnt << "\n";
                        cnt = 0;
                        for(int k = 0; k < width; k++) {
                            if(second_row & ((bit_board)1 << ((j+1)*10 + k))) {
                                cnt++;    
                            }
                        }
                        if(cnt <= 3) {
                            continue;
                        }
                        std::cout << "Count2: " << cnt << "\n";
                    }
                    return true;
                } 
            }
        }
    }
    return false;
}

bool AnalysisContext::Prune(search_node& node) {
    bit_board row = 0b1111111111;
    bit_board above = row << 10;
    for(int i = 0; i < 19; i++) {
        bit_board check = ~((row & node.board) << 10) & (above & node.board);
        if(check) {
            for(int j = 0; j < 10; j++) {
                // Check is the combination of two rows, shifted up one row
                if(((bit_board)1 << (j + (i+1)*10)) & check) {
                    bool tspin = false;
                    // block to left is filled
                    if(j >= 1 && (bit_board)1 << ((j-1) + i*10) & node.board) {
                        tspin = tspin | is_tspin_shape(node.board, j, j+3, i-1, i+2, false);
                    }
                    // block to right is filled
                    if(j <= 8 && (bit_board)1 << ((j+1) + i*10) & node.board) {
                        tspin = tspin | is_tspin_shape(node.board, j-2, j+1, i-1, i+2, false);
                    }
                    bool donation = is_tspin_shape(node.board, 0, 10, i, i+4, false);
                    if(!tspin && !donation) {
                        pruned++;
                        return true;
                    }
                }
            }
        }
        row <<= 10;
        above <<= 10;
    }
    return false;
}

bool AnalysisContext::DFS(search_node node) {
    // Add some metadata to the transposition matrix entry
    // Top 56 bits are 0 anyway so we can add whatever we want here
    bit_board transposition = node.board 
        | ((bit_board)(node.attack&0xff) << 248) 
        | ((bit_board)(node.curr_idx&0xff) << 240) 
        | ((bit_board)(node.back_to_back&0xff) << 232)
        | ((bit_board)(node.combo&0xff) << 224)
        | ((bit_board)(node.depth&0xff) << 216);
    if(transposition_matrix.count(transposition) != 0) {
        return false;
    }
    transposition_matrix.insert(transposition);
    if(Prune(node)) {
        return false;
    }
    if(node.depth == 0 || node.curr_idx >= mino_sequence.size()) {
        int score = Evaluate(node);
        if(score > best_score) {
            // TODO: Check board is actually reachable

            // Clear the best move sequence since our parent calls will update it
            std::cout << node.board << "\n";
            best_score = score;
            best_moves = curr_moves;
            for(int i = 19; i >= 0; i--) {
                for(int j = 0; j < 10; j++) {
                    if(node.board & ((bit_board)1 << (i*10 + j))) {
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
    std::vector<move_info> possible_moves = FindPossiblePositions(node.board, mino_sequence[node.curr_idx]);
    bool found_new_best = false;
    for(auto next_move : possible_moves) {
        search_node new_node;
        new_node.board = node.board | MoveToBitBoard(next_move);
        new_node.back_to_back = node.back_to_back;
        new_node.combo = node.combo;
        // This function updates combo and back_to_back
        new_node.attack = node.attack + update_board(new_node.board, next_move, new_node.back_to_back, new_node.combo);
        new_node.back_to_back_lost = node.back_to_back && !new_node.back_to_back;
        new_node.depth = node.depth - 1;
        new_node.curr_idx = node.curr_idx + 1;
        curr_moves.push_back(next_move);
        DFS(new_node);
        curr_moves.pop_back();
    }
    return found_new_best;
}

// This code is extremely performance sensitive
int AnalysisContext::Evaluate(const search_node& node) {
    int score = node.attack*10;

    // Check for overhangs/holes
    // Each is worth -2 attack
    //
    // Bitwise approach instead of looping over every bit decreases time from .73s to .2s
    // on a three depth search.
    bit_board row = 0b1111111111;
    bit_board above = row << 10;
    std::vector<int> row_heights = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
    for(int i = 0; i < 19; i++) {
        bit_board check = ~((row & node.board) << 10) & (above & node.board);
        // We shouldn't hit this check very often
        if(check) {
            for(int j = 0; j < 10; j++) {
                // Check is the combination of two rows, shifted up one row
                if(((bit_board)1 << (j + (i+1)*10)) & check) {
                    for(int k = i+1; k < 19; k++) {
                        // Each block on top of a hole is counted
                        if((bit_board)1 << (j + (k*10)) & node.board) {
                            score -= 20;
                        }
                    }
                }
            }
        }
        bit_board height_check = row & node.board;
        if(height_check) {
            for(int j = 0; j < 10; j++) {
                if(1<<(j + i*10) & height_check) {
                    row_heights[j] = i;
                }
            }
        }
        row <<= 10;
        above <<= 10;
    }

    int max_diff = 0;
    // Reduce score for jagged boards
    for(int i = 0; i < 9; i++) {
        int dist = std::abs(row_heights[i]-row_heights[i+1]);
        score -= dist*dist;
    }

    row_heights.insert(row_heights.begin(), 20);
    row_heights.push_back(20);
    int well = -1;
    int well_sides = 0;
    bool true_well_found = false;
    // Find well
    for(int i = 1; i < 11; i++) {
        if(row_heights[i] > row_heights[i+1] || row_heights[i] > row_heights[i-1]) {
            continue;
        }
        int dist1 = std::abs(row_heights[i]-row_heights[i+1]);
        int dist2 = std::abs(row_heights[i-1]-row_heights[i]);
        if(dist1 >= 3 && dist2 >= 3) {
            if(true_well_found) {
                // Double well
                score -= 60;
            }
            well_sides = dist1 + dist2;
            true_well_found = true;
            well = i-1;
        }
        if(dist1 + dist2 > well_sides && !true_well_found) {
            well = i-1;
            well_sides = dist1 + dist2;
        }
    }

    row_heights.erase(row_heights.begin());
    row_heights.pop_back();

    int add_back = 0;
    for(int i = well-1; i <= well+1; i++) {
        if(i < 0 || i >= 10) {
            continue;
        }
        int dist = std::abs(row_heights[i] - row_heights[well]);
        add_back += dist*dist;
    }
    // Don't subtract score for a well
    score += add_back;

    // Back to back loss worth -2 attack
    // This will probably give all initial boards a negative score
    if(node.back_to_back_lost) {
        score -= 20;
    }

    // Center of t-piece
    // If this isn't empty, don't bother doing more work
    const bit_board center = 0b000 | (0b010 << 10) | (0b000 << 20);

    // T shapes
    const bit_board t0 = 0b000 | (0b111 << 10) | (0b010 << 20);
    const bit_board t1 = 0b010 | (0b110 << 10) | (0b010 << 20);
    const bit_board t2 = 0b010 | (0b111 << 10) | (0b000 << 20);
    const bit_board t3 = 0b010 | (0b011 << 10) | (0b010 << 20);

    // T-spin corner possibilities
    const bit_board corner0 = 0b001 | (0b000 << 10) | (0b101 << 20);
    const bit_board corner1 = 0b100 | (0b000 << 10) | (0b101 << 20);
    const bit_board corner2 = 0b101 | (0b000 << 10) | (0b001 << 20);
    const bit_board corner3 = 0b101 | (0b000 << 10) | (0b100 << 20);
    
    if(quiescence) {
        // TODO: Check for the shape of a tpiece for quiescence search
        if(is_tspin_shape(node.board, 0, 11, 0, 21, true)) {
            quiescence_node q_node;
            q_node.node = node;
            q_node.init_state = curr_moves;
            quiescence_list.push_back(q_node);
        }
    }

    return score;
}
