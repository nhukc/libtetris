#include "bot/MoveGen.hpp"
#include <iostream>
#include <bits/stdc++.h>

// Mino positional hash
struct MinoHash { 
public: 
    // Somewhat reasonable hash function
    size_t operator()(const tetris::Mino* t) const
    { 
        std::vector<std::vector<int>> filled_coords = t->GetFilledCoordinates();
        size_t h1 = 0;
        size_t h2 = 0;
        for(int i = 0; i < filled_coords.size(); i++) {
            h1 += filled_coords[i][0];
            h1 *= 37;
            h2 += filled_coords[i][1];
            h2 *= 37;
        }
        std::cout << (h1^h2) << "\n";
        return h1 ^ h2;
    } 
}; 

struct MinoPtrEquality {
  bool operator()(const tetris::Mino *Lhs, const tetris::Mino *Rhs) const {
    return *Lhs == *Rhs;
  }
};

std::vector<std::unique_ptr<tetris::Mino>> FindPossiblePositions(tetris::Board board, tetris::Mino& mino) {
    // Locations of highest block in column
    // Used to prune boards with floating minos
    std::vector<int> column_heights;
    for(int i = 0; i < board.width; i++) {
        bool found = false;
        for(int j = board.height-1; j >= 0; j--) {
            if(board.GetSquare(i, j) != tetris::Tile::Empty) {
                column_heights.push_back(j);
                found = true;
                break;
            }
        }
        if(!found) {
            column_heights.push_back(-1);
        }
    }
    std::unordered_set<tetris::Mino*, MinoHash, MinoPtrEquality> seen;
    std::vector<std::unique_ptr<tetris::Mino>> positions;
    for(int r = 0; r < 4; r++) {
        for(int i = -mino.bb_w; i < board.width; i++) {
            // Find max height the mino can be placed at
            int max_height = -1;
            for(int j = 0; j < mino.bb_w; j++) {
                max_height = std::max(column_heights[j], max_height);
            }
            // Start at max_height + bb_h since mino is positioned by top left corner of bounding box
            for(int j = max_height + mino.bb_h; j >= 0; j--) {
                tetris::Mino* mino_copy = mino.Clone();
                mino_copy->x = i;
                mino_copy->y = j;
                if(seen.count(mino_copy) > 0) {
                    continue;
                }
                if(mino_copy->Colliding(board)) {
                    continue;
                }
                // Ensure mino is on a surface
                mino_copy->x = i;
                mino_copy->y = j-1;
                if(!mino_copy->Colliding(board)) {
                    continue;
                }
                mino_copy->x = i;
                mino_copy->y = j;

                seen.insert(mino_copy);
                positions.push_back(std::unique_ptr<tetris::Mino>(mino_copy));
                // Does not check if underground position is reachable for performance reasons (kicks, tspins, etc)
                // Filter out these cases manually later in pipeline
            }
        }
        mino.InternalRotate(tetris::Rotation::Clockwise);
    }
    return positions;
}
