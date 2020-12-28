#ifndef TETRIS_HPP
#define TETRIS_HPP

#include <assert.h>
#include <bitset>
#include <map>
#include <string>
#include <vector>

namespace tetris {

enum Tile { I, S, Z, O, J, L, T, Empty, X };
enum Rotation { Clockwise, CounterClockwise, OneEighty };

class Action;

// Efficiently small tetris board
struct Board {
    private:
    std::vector<Tile> board_; 

    public:
	int width;
	int height;

    Board(int w, int h) : width(w), height(h) {
        for(int i = 0; i < w; i++) {
            for(int j = 0; j < h; j++) {
                board_.push_back(Tile::Empty);
            }
        }
    };

    // Changes the value of a single board square
    // As specified in the guideline, y=0 is the bottom of the board
    void FillSquare(int x, int y, Tile t);

    // Gets the value of a single board square
    // As specified in the guideline, y=0 is the bottom of the board
    Tile GetSquare(int x, int y) const;

    // Detects if lines should be cleared and updates the board
    // returns the number of lines cleared
    int UpdateBoard();

    std::string to_string() const; 
};

std::ostream &operator<<(std::ostream &os, const Board &b);

// Represents the action of placing one mino to the board
class Action {
    protected:

    // size of bounding box
    int bb_w, bb_h;
    
    // bounding box of the piece
    // used to compute rotations
    // edges of bounding box may exist outside of board
    std::vector<std::vector<Tile>> bounding_box_;
   
    // Map of kicks for rotations
    // Each key is of the form (original orientation, new orientation)
    // The values describe possible translations if rotation causes collisions
    // If none of these kicks work, then rotation is impossible
    std::map<std::pair<int, int>, std::vector<std::pair<int, int>>> kick_table_;

    // Pieces lock after harddrop or after staying still for the lock-delay time
    bool locked_;

    // Board used to detect collisions when rotating/translating
    Board& board_;

    // Translates the mino without collision checking
    void InternalTranslate(int x, int y);

    // Rotates the mino without collision checking
    void InternalRotate(Rotation r);

    public:

    // location of top left corner of bounding box with respect to board
    int x, y;
    
    // current orientation of piece
    // 0 - original
    // 1 - cw turned once
    // used to compute kicks
    int orientation_ = 0;

    struct RotationContext {
        // If rotation was possible
        bool rotated;
        // If we required a kick to rotate
        bool kicked;
    };

    Action(Board& board) : board_(board) {
        // TODO: Determine where pieces should spawn
    };
    
    // Gets the Tile represented by this mino
    virtual Tile GetTile() = 0;

    // Translates this mino with collisions detected in context of board
    // returns -1 if failed
    bool Translate(int x, int y);

    // Rotates this mino with collisions detected in context of board
    // returns -1 if failed
    RotationContext Rotate(Rotation r);

    // Checks if the mino overlaps with any non-empty squares in the board
    // or if the mino is partially outside the board.
    // Returns true if the mino collides with walls or non-empty squares
    // Otherwise, returns false
    bool Colliding();

    // Check if the piece is locked already
    bool IsLocked() { return locked_; }

    // Check if current placement of mino would count a t-spin
    virtual bool IsTSpin() { return false; };
    
    // Check if current placement of mino would count a t-spin
    virtual bool IsMiniTSpin() { return false; };
    
    // Overwrites board squares with this mino
    void ApplyToBoard();
    
    std::string to_string() const; 
};

std::ostream &operator<<(std::ostream &os, const Action &b);

class OAction : public Action {
    public:
    OAction(Board& board) : Action(board) {
        x = 4;
        y = board_.height-1;

        bb_w = 2;
        bb_h = 2;

        bounding_box_ = std::vector<std::vector<Tile>>({
            {Tile::O, Tile::O}, 
            {Tile::O, Tile::O}
        });
        
        // O piece has no kicks
        kick_table_ = std::map<std::pair<int, int>, std::vector<std::pair<int, int>>>();
        kick_table_.insert(std::make_pair(std::make_pair(0,1), std::vector<std::pair<int,int>> ({ {0,0} })));
        kick_table_.insert(std::make_pair(std::make_pair(1,0), std::vector<std::pair<int,int>> ({ {0,0} })));
        kick_table_.insert(std::make_pair(std::make_pair(1,2), std::vector<std::pair<int,int>> ({ {0,0} })));
        kick_table_.insert(std::make_pair(std::make_pair(2,1), std::vector<std::pair<int,int>> ({ {0,0} })));
        kick_table_.insert(std::make_pair(std::make_pair(2,3), std::vector<std::pair<int,int>> ({ {0,0} })));
        kick_table_.insert(std::make_pair(std::make_pair(3,2), std::vector<std::pair<int,int>> ({ {0,0} })));
        kick_table_.insert(std::make_pair(std::make_pair(3,0), std::vector<std::pair<int,int>> ({ {0,0} })));
        kick_table_.insert(std::make_pair(std::make_pair(0,3), std::vector<std::pair<int,int>> ({ {0,0} })));
    }

    virtual Tile GetTile() { return Tile::O; }
};

class LAction : public Action {
    public:
    LAction(Board& board) : Action(board) {
        x = 3;
        y = board_.height-1;

        bb_w = 3;
        bb_h = 3;

        bounding_box_ = std::vector<std::vector<Tile>>({
            {Tile::Empty, Tile::Empty, Tile::L}, 
            {Tile::L,     Tile::L,     Tile::L}, 
            {Tile::Empty, Tile::Empty, Tile::Empty}
        });

        // 0 = spawn state
        // R = state resulting from a clockwise rotation ("right") from spawn
        // L = state resulting from a counter-clockwise ("left") rotation from spawn
        // 2 = state resulting from 2 successive rotations in either direction from spawn.

        // J, L, S, T, Z Tetromino Wall Kick Data
        //      Test 1	Test 2	Test 3	Test 4	Test 5
        // 0->R	( 0, 0)	(-1, 0)	(-1,+1)	( 0,-2)	(-1,-2)
        // R->0	( 0, 0)	(+1, 0)	(+1,-1)	( 0,+2)	(+1,+2)
        // R->2	( 0, 0)	(+1, 0)	(+1,-1)	( 0,+2)	(+1,+2)
        // 2->R	( 0, 0)	(-1, 0)	(-1,+1)	( 0,-2)	(-1,-2)
        // 2->L	( 0, 0)	(+1, 0)	(+1,+1)	( 0,-2)	(+1,-2)
        // L->2	( 0, 0)	(-1, 0)	(-1,-1)	( 0,+2)	(-1,+2)
        // L->0	( 0, 0)	(-1, 0)	(-1,-1)	( 0,+2)	(-1,+2)
        // 0->L	( 0, 0)	(+1, 0)	(+1,+1)	( 0,-2)	(+1,-2)

        kick_table_ = std::map<std::pair<int, int>, std::vector<std::pair<int, int>>>();
        kick_table_.insert(std::make_pair(std::make_pair(0,1), std::vector<std::pair<int,int>> ({ {0,0}, {-1,0}, {-1,1}, {0,-2}, {-1,-2} })));
        kick_table_.insert(std::make_pair(std::make_pair(1,0), std::vector<std::pair<int,int>> ({ {0,0}, {1,0}, {1,-1}, {0,2}, {1,2} })));
        kick_table_.insert(std::make_pair(std::make_pair(1,2), std::vector<std::pair<int,int>> ({ {0,0}, {1,0}, {1,-1}, {0,2}, {1,2} })));
        kick_table_.insert(std::make_pair(std::make_pair(2,1), std::vector<std::pair<int,int>> ({ {0,0}, {-1,0}, {-1,1}, {0,-2}, {-1,-2} })));
        kick_table_.insert(std::make_pair(std::make_pair(2,3), std::vector<std::pair<int,int>> ({ {0,0}, {1,0}, {1,1}, {0,-2}, {1,-2} })));
        kick_table_.insert(std::make_pair(std::make_pair(3,2), std::vector<std::pair<int,int>> ({ {0,0}, {-1,0}, {-1,-1}, {0,2}, {-1,2} })));
        kick_table_.insert(std::make_pair(std::make_pair(3,0), std::vector<std::pair<int,int>> ({ {0,0}, {-1,0}, {-1,-1}, {0,2}, {-1,2} })));
        kick_table_.insert(std::make_pair(std::make_pair(0,3), std::vector<std::pair<int,int>> ({ {0,0}, {1,0}, {1,1}, {0,-2}, {1,-2} })));
    }
    
    virtual Tile GetTile() { return Tile::L; }
};

class JAction : public Action {
    public:
    JAction(Board& board) : Action(board) {
        x = 3;
        y = board_.height-1;

        bb_w = 3;
        bb_h = 3;

        bounding_box_ = std::vector<std::vector<Tile>>({
            {Tile::J,     Tile::Empty, Tile::Empty}, 
            {Tile::J,     Tile::J,     Tile::J}, 
            {Tile::Empty, Tile::Empty, Tile::Empty}
        });

        // 0 = spawn state
        // R = state resulting from a clockwise rotation ("right") from spawn
        // L = state resulting from a counter-clockwise ("left") rotation from spawn
        // 2 = state resulting from 2 successive rotations in either direction from spawn.

        // J, L, S, T, Z Tetromino Wall Kick Data
        //      Test 1	Test 2	Test 3	Test 4	Test 5
        // 0->R	( 0, 0)	(-1, 0)	(-1,+1)	( 0,-2)	(-1,-2)
        // R->0	( 0, 0)	(+1, 0)	(+1,-1)	( 0,+2)	(+1,+2)
        // R->2	( 0, 0)	(+1, 0)	(+1,-1)	( 0,+2)	(+1,+2)
        // 2->R	( 0, 0)	(-1, 0)	(-1,+1)	( 0,-2)	(-1,-2)
        // 2->L	( 0, 0)	(+1, 0)	(+1,+1)	( 0,-2)	(+1,-2)
        // L->2	( 0, 0)	(-1, 0)	(-1,-1)	( 0,+2)	(-1,+2)
        // L->0	( 0, 0)	(-1, 0)	(-1,-1)	( 0,+2)	(-1,+2)
        // 0->L	( 0, 0)	(+1, 0)	(+1,+1)	( 0,-2)	(+1,-2)

        kick_table_ = std::map<std::pair<int, int>, std::vector<std::pair<int, int>>>();
        kick_table_.insert(std::make_pair(std::make_pair(0,1), std::vector<std::pair<int,int>> ({ {0,0}, {-1,0}, {-1,1}, {0,-2}, {-1,-2} })));
        kick_table_.insert(std::make_pair(std::make_pair(1,0), std::vector<std::pair<int,int>> ({ {0,0}, {1,0}, {1,-1}, {0,2}, {1,2} })));
        kick_table_.insert(std::make_pair(std::make_pair(1,2), std::vector<std::pair<int,int>> ({ {0,0}, {1,0}, {1,-1}, {0,2}, {1,2} })));
        kick_table_.insert(std::make_pair(std::make_pair(2,1), std::vector<std::pair<int,int>> ({ {0,0}, {-1,0}, {-1,1}, {0,-2}, {-1,-2} })));
        kick_table_.insert(std::make_pair(std::make_pair(2,3), std::vector<std::pair<int,int>> ({ {0,0}, {1,0}, {1,1}, {0,-2}, {1,-2} })));
        kick_table_.insert(std::make_pair(std::make_pair(3,2), std::vector<std::pair<int,int>> ({ {0,0}, {-1,0}, {-1,-1}, {0,2}, {-1,2} })));
        kick_table_.insert(std::make_pair(std::make_pair(3,0), std::vector<std::pair<int,int>> ({ {0,0}, {-1,0}, {-1,-1}, {0,2}, {-1,2} })));
        kick_table_.insert(std::make_pair(std::make_pair(0,3), std::vector<std::pair<int,int>> ({ {0,0}, {1,0}, {1,1}, {0,-2}, {1,-2} })));
    }
    
    virtual Tile GetTile() { return Tile::J; }
};

class SAction : public Action {
    public:
    SAction(Board& board) : Action(board) {
        x = 3;
        y = board_.height-1;

        bb_w = 3;
        bb_h = 3;

        bounding_box_ = std::vector<std::vector<Tile>>({
            {Tile::Empty, Tile::S,     Tile::S}, 
            {Tile::S,     Tile::S,     Tile::Empty}, 
            {Tile::Empty, Tile::Empty, Tile::Empty}
        });

        // 0 = spawn state
        // R = state resulting from a clockwise rotation ("right") from spawn
        // L = state resulting from a counter-clockwise ("left") rotation from spawn
        // 2 = state resulting from 2 successive rotations in either direction from spawn.

        // J, L, S, T, Z Tetromino Wall Kick Data
        //      Test 1	Test 2	Test 3	Test 4	Test 5
        // 0->R	( 0, 0)	(-1, 0)	(-1,+1)	( 0,-2)	(-1,-2)
        // R->0	( 0, 0)	(+1, 0)	(+1,-1)	( 0,+2)	(+1,+2)
        // R->2	( 0, 0)	(+1, 0)	(+1,-1)	( 0,+2)	(+1,+2)
        // 2->R	( 0, 0)	(-1, 0)	(-1,+1)	( 0,-2)	(-1,-2)
        // 2->L	( 0, 0)	(+1, 0)	(+1,+1)	( 0,-2)	(+1,-2)
        // L->2	( 0, 0)	(-1, 0)	(-1,-1)	( 0,+2)	(-1,+2)
        // L->0	( 0, 0)	(-1, 0)	(-1,-1)	( 0,+2)	(-1,+2)
        // 0->L	( 0, 0)	(+1, 0)	(+1,+1)	( 0,-2)	(+1,-2)

        kick_table_ = std::map<std::pair<int, int>, std::vector<std::pair<int, int>>>();
        kick_table_.insert(std::make_pair(std::make_pair(0,1), std::vector<std::pair<int,int>> ({ {0,0}, {-1,0}, {-1,1}, {0,-2}, {-1,-2} })));
        kick_table_.insert(std::make_pair(std::make_pair(1,0), std::vector<std::pair<int,int>> ({ {0,0}, {1,0}, {1,-1}, {0,2}, {1,2} })));
        kick_table_.insert(std::make_pair(std::make_pair(1,2), std::vector<std::pair<int,int>> ({ {0,0}, {1,0}, {1,-1}, {0,2}, {1,2} })));
        kick_table_.insert(std::make_pair(std::make_pair(2,1), std::vector<std::pair<int,int>> ({ {0,0}, {-1,0}, {-1,1}, {0,-2}, {-1,-2} })));
        kick_table_.insert(std::make_pair(std::make_pair(2,3), std::vector<std::pair<int,int>> ({ {0,0}, {1,0}, {1,1}, {0,-2}, {1,-2} })));
        kick_table_.insert(std::make_pair(std::make_pair(3,2), std::vector<std::pair<int,int>> ({ {0,0}, {-1,0}, {-1,-1}, {0,2}, {-1,2} })));
        kick_table_.insert(std::make_pair(std::make_pair(3,0), std::vector<std::pair<int,int>> ({ {0,0}, {-1,0}, {-1,-1}, {0,2}, {-1,2} })));
        kick_table_.insert(std::make_pair(std::make_pair(0,3), std::vector<std::pair<int,int>> ({ {0,0}, {1,0}, {1,1}, {0,-2}, {1,-2} })));
    }

    virtual Tile GetTile() { return Tile::S; }
};

class ZAction : public Action {
    public:
    ZAction(Board& board) : Action(board) {
        x = 3;
        y = board_.height-1;

        bb_w = 3;
        bb_h = 3;

        bounding_box_ = std::vector<std::vector<Tile>>({
            {Tile::Z,     Tile::Z,     Tile::Empty}, 
            {Tile::Empty, Tile::Z,     Tile::Z}, 
            {Tile::Empty, Tile::Empty, Tile::Empty}
        });

        // 0 = spawn state
        // R = state resulting from a clockwise rotation ("right") from spawn
        // L = state resulting from a counter-clockwise ("left") rotation from spawn
        // 2 = state resulting from 2 successive rotations in either direction from spawn.

        // J, L, S, T, Z Tetromino Wall Kick Data
        //      Test 1	Test 2	Test 3	Test 4	Test 5
        // 0->R	( 0, 0)	(-1, 0)	(-1,+1)	( 0,-2)	(-1,-2)
        // R->0	( 0, 0)	(+1, 0)	(+1,-1)	( 0,+2)	(+1,+2)
        // R->2	( 0, 0)	(+1, 0)	(+1,-1)	( 0,+2)	(+1,+2)
        // 2->R	( 0, 0)	(-1, 0)	(-1,+1)	( 0,-2)	(-1,-2)
        // 2->L	( 0, 0)	(+1, 0)	(+1,+1)	( 0,-2)	(+1,-2)
        // L->2	( 0, 0)	(-1, 0)	(-1,-1)	( 0,+2)	(-1,+2)
        // L->0	( 0, 0)	(-1, 0)	(-1,-1)	( 0,+2)	(-1,+2)
        // 0->L	( 0, 0)	(+1, 0)	(+1,+1)	( 0,-2)	(+1,-2)

        kick_table_ = std::map<std::pair<int, int>, std::vector<std::pair<int, int>>>();
        kick_table_.insert(std::make_pair(std::make_pair(0,1), std::vector<std::pair<int,int>> ({ {0,0}, {-1,0}, {-1,1}, {0,-2}, {-1,-2} })));
        kick_table_.insert(std::make_pair(std::make_pair(1,0), std::vector<std::pair<int,int>> ({ {0,0}, {1,0}, {1,-1}, {0,2}, {1,2} })));
        kick_table_.insert(std::make_pair(std::make_pair(1,2), std::vector<std::pair<int,int>> ({ {0,0}, {1,0}, {1,-1}, {0,2}, {1,2} })));
        kick_table_.insert(std::make_pair(std::make_pair(2,1), std::vector<std::pair<int,int>> ({ {0,0}, {-1,0}, {-1,1}, {0,-2}, {-1,-2} })));
        kick_table_.insert(std::make_pair(std::make_pair(2,3), std::vector<std::pair<int,int>> ({ {0,0}, {1,0}, {1,1}, {0,-2}, {1,-2} })));
        kick_table_.insert(std::make_pair(std::make_pair(3,2), std::vector<std::pair<int,int>> ({ {0,0}, {-1,0}, {-1,-1}, {0,2}, {-1,2} })));
        kick_table_.insert(std::make_pair(std::make_pair(3,0), std::vector<std::pair<int,int>> ({ {0,0}, {-1,0}, {-1,-1}, {0,2}, {-1,2} })));
        kick_table_.insert(std::make_pair(std::make_pair(0,3), std::vector<std::pair<int,int>> ({ {0,0}, {1,0}, {1,1}, {0,-2}, {1,-2} })));
    }

    virtual Tile GetTile() { return Tile::Z; }
};

class IAction : public Action {
    public:
    IAction(Board& board) : Action(board) {
        x = 3;
        y = board_.height-1;

        bb_w = 4;
        bb_h = 4;

        bounding_box_ = std::vector<std::vector<Tile>>({
            {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty},
            {Tile::I,     Tile::I,     Tile::I,     Tile::I}, 
            {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty}, 
            {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty},
        });
        
        // 0 = spawn state
        // R = state resulting from a clockwise rotation ("right") from spawn
        // L = state resulting from a counter-clockwise ("left") rotation from spawn
        // 2 = state resulting from 2 successive rotations in either direction from spawn.

        // I Tetromino Wall Kick Data
        //      Test 1	Test 2	Test 3	Test 4	Test 5
        // 0->R	( 0, 0)	(-2, 0)	(+1, 0)	(-2,-1)	(+1,+2)
        // R->0	( 0, 0)	(+2, 0)	(-1, 0)	(+2,+1)	(-1,-2)
        // R->2	( 0, 0)	(-1, 0)	(+2, 0)	(-1,+2)	(+2,-1)
        // 2->R	( 0, 0)	(+1, 0)	(-2, 0)	(+1,-2)	(-2,+1)
        // 2->L	( 0, 0)	(+2, 0)	(-1, 0)	(+2,+1)	(-1,-2)
        // L->2	( 0, 0)	(-2, 0)	(+1, 0)	(-2,-1)	(+1,+2)
        // L->0	( 0, 0)	(+1, 0)	(-2, 0)	(+1,-2)	(-2,+1)
        // 0->L	( 0, 0)	(-1, 0)	(+2, 0)	(-1,+2)	(+2,-1)
        
        kick_table_ = std::map<std::pair<int, int>, std::vector<std::pair<int, int>>>();
        kick_table_.insert(std::make_pair(std::make_pair(0,1), std::vector<std::pair<int,int>> ({ {0,0}, {-2,0}, {1,0}, {-2,-1}, {1,2} })));
        kick_table_.insert(std::make_pair(std::make_pair(1,0), std::vector<std::pair<int,int>> ({ {0,0}, {2,0}, {-1,0}, {2,1}, {-1,-2} })));
        kick_table_.insert(std::make_pair(std::make_pair(1,2), std::vector<std::pair<int,int>> ({ {0,0}, {-1,0}, {2,0}, {-1,2}, {2,-1} })));
        kick_table_.insert(std::make_pair(std::make_pair(2,1), std::vector<std::pair<int,int>> ({ {0,0}, {1,0}, {-2,0}, {1,-2}, {-2,1} })));
        kick_table_.insert(std::make_pair(std::make_pair(2,3), std::vector<std::pair<int,int>> ({ {0,0}, {2,0}, {-1,0}, {2,1}, {-1,-2} })));
        kick_table_.insert(std::make_pair(std::make_pair(3,2), std::vector<std::pair<int,int>> ({ {0,0}, {-2,0}, {1,0}, {-2,-1}, {1,2} })));
        kick_table_.insert(std::make_pair(std::make_pair(3,0), std::vector<std::pair<int,int>> ({ {0,0}, {1,0}, {-2,0}, {1,-2}, {-2,1} })));
        kick_table_.insert(std::make_pair(std::make_pair(0,3), std::vector<std::pair<int,int>> ({ {0,0}, {-1,0}, {2,0}, {-1,2}, {2,-1} })));
    }

    virtual Tile GetTile() { return Tile::I; }
};

class TAction : public Action {
    public:
    TAction(Board& board) : Action(board) {
        x = 3;
        y = board_.height-1;

        bb_w = 3;
        bb_h = 3;

        bounding_box_ = std::vector<std::vector<Tile>>({
            {Tile::Empty, Tile::T,     Tile::Empty}, 
            {Tile::T,     Tile::T,     Tile::T}, 
            {Tile::Empty, Tile::Empty, Tile::Empty}
        });

        // 0 = spawn state
        // R = state resulting from a clockwise rotation ("right") from spawn
        // L = state resulting from a counter-clockwise ("left") rotation from spawn
        // 2 = state resulting from 2 successive rotations in either direction from spawn.

        // J, L, S, T, Z Tetromino Wall Kick Data
        //      Test 1	Test 2	Test 3	Test 4	Test 5
        // 0->R	( 0, 0)	(-1, 0)	(-1,+1)	( 0,-2)	(-1,-2)
        // R->0	( 0, 0)	(+1, 0)	(+1,-1)	( 0,+2)	(+1,+2)
        // R->2	( 0, 0)	(+1, 0)	(+1,-1)	( 0,+2)	(+1,+2)
        // 2->R	( 0, 0)	(-1, 0)	(-1,+1)	( 0,-2)	(-1,-2)
        // 2->L	( 0, 0)	(+1, 0)	(+1,+1)	( 0,-2)	(+1,-2)
        // L->2	( 0, 0)	(-1, 0)	(-1,-1)	( 0,+2)	(-1,+2)
        // L->0	( 0, 0)	(-1, 0)	(-1,-1)	( 0,+2)	(-1,+2)
        // 0->L	( 0, 0)	(+1, 0)	(+1,+1)	( 0,-2)	(+1,-2)

        kick_table_ = std::map<std::pair<int, int>, std::vector<std::pair<int, int>>>();
        kick_table_.insert(std::make_pair(std::make_pair(0,1), std::vector<std::pair<int,int>> ({ {0,0}, {-1,0}, {-1,1}, {0,-2}, {-1,-2} })));
        kick_table_.insert(std::make_pair(std::make_pair(1,0), std::vector<std::pair<int,int>> ({ {0,0}, {1,0}, {1,-1}, {0,2}, {1,2} })));
        kick_table_.insert(std::make_pair(std::make_pair(1,2), std::vector<std::pair<int,int>> ({ {0,0}, {1,0}, {1,-1}, {0,2}, {1,2} })));
        kick_table_.insert(std::make_pair(std::make_pair(2,1), std::vector<std::pair<int,int>> ({ {0,0}, {-1,0}, {-1,1}, {0,-2}, {-1,-2} })));
        kick_table_.insert(std::make_pair(std::make_pair(2,3), std::vector<std::pair<int,int>> ({ {0,0}, {1,0}, {1,1}, {0,-2}, {1,-2} })));
        kick_table_.insert(std::make_pair(std::make_pair(3,2), std::vector<std::pair<int,int>> ({ {0,0}, {-1,0}, {-1,-1}, {0,2}, {-1,2} })));
        kick_table_.insert(std::make_pair(std::make_pair(3,0), std::vector<std::pair<int,int>> ({ {0,0}, {-1,0}, {-1,-1}, {0,2}, {-1,2} })));
        kick_table_.insert(std::make_pair(std::make_pair(0,3), std::vector<std::pair<int,int>> ({ {0,0}, {1,0}, {1,1}, {0,-2}, {1,-2} })));
    }
    
    virtual Tile GetTile() { return Tile::T; }
    
    virtual bool IsTSpin();
    virtual bool IsMiniTSpin();
};

} // namespace tetris

#endif
