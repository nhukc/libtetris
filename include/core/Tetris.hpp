#ifndef TETRIS_HPP
#define TETRIS_HPP

#include <assert.h>
#include <array>
#include <bitset>
#include <map>
#include <string>
#include <vector>

namespace tetris {

enum Tile { I, S, Z, O, J, L, T, Empty, X };
enum Rotation { Clockwise, CounterClockwise };

class Mino;

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

    // Apply a mino to the board
    void ApplyMino(tetris::Mino& mino);

    // Detects if lines should be cleared and updates the board
    // returns the number of lines cleared
    int UpdateBoard();

    std::string to_string() const; 
};

std::ostream &operator<<(std::ostream &os, const Board &b);
    
// Struct returned when attempting to rotate a mino
struct RotationContext {
    // If rotation was possible
    bool rotated;
    // If we required a kick to rotate
    bool kicked;
};

// Wrapper class to avoid multidimensional vectors
struct PointVec {
    public:
    // Holds at most 8 points
    int vec[2*8];
    int size = 0;

    PointVec() {}

    // Get ith point, coordinate j
    // 0 <= i < length
    // 0 <= j <= 1
    inline int Get(int i, int j) {
        return vec[i*2 + j];
    }

    inline void Set(int i, int j, int x) {
        vec[i*2 + j] = x;
    }

    inline void Push(int x, int y) {
        vec[2*size] = x;
        vec[2*size+1] = y;
        size++;
    }

    inline int Size() {
        return size;
    }

    inline bool operator==(const PointVec& rhs){ 
        for(int i = 0; i < size*2; i++) {
            if(vec[i] != rhs.vec[i])
                return false;
        }
        return true;
    }
};

// Represents the action of placing one mino to the board
class Mino {
    protected:

    // Map of kicks for rotations
    // Each key is of the form (original orientation, new orientation)
    // The values describe possible translations if rotation causes collisions
    // If none of these kicks work, then rotation is impossible
    std::map<std::pair<int, int>, std::vector<std::pair<int, int>>> kick_table_;

    tetris::Tile tile_type;

    public:

    // location of top left corner of bounding box with respect to board
    int x, y;
    
    // size of bounding box
    int bb_w, bb_h;
    
    // bounding box of the piece
    // used to compute rotations
    // edges of bounding box may exist outside of board
    std::vector<std::vector<Tile>> bounding_box_; 
    
    // current orientation of piece
    // 0 - original
    // 1 - cw turned once
    // used to compute kicks
    int orientation_ = 0;
    
    Mino(int x, int y) : x(x), y(y) {};

    // Create mino in default position of board
    Mino(Board& board);
 
    // Gets the Tile represented by this mino
    virtual Tile GetTile() = 0;
    
    // Translates the mino without collision checking
    void InternalTranslate(int x, int y);

    // Rotates the mino without collision checking
    void InternalRotate(Rotation r);

    // Translates this mino with collisions detected in context of board
    // returns -1 if failed
    bool Translate(int x, int y, Board& board);

    // Rotates this mino with collisions detected in context of board
    // returns -1 if failed
    RotationContext Rotate(Rotation r, Board& board);

    // Checks if the mino overlaps with any non-empty squares in the board
    // or if the mino is partially outside the board.
    // Returns true if the mino collides with walls or non-empty squares
    // Otherwise, returns false
    bool Colliding(Board& board); 

    // Returns an vector of 2-tuples representing which x/y positions would be filled by this piece
    inline PointVec GetFilledCoordinates() const;
    inline PointVec GetFilledCoordinatesSlow() const;

    // Check if current placement of mino would count a t-spin
    virtual bool IsTSpin() { return false; };
    
    // Check if current placement of mino would count a t-spin
    virtual bool IsMiniTSpin() { return false; };

    virtual Mino* Clone() = 0;

    // Checks if minos are positionally identical
    // Returns true iff both minos occupy same position (not necessarily orientation or x/y)
    bool operator==(const Mino& t) const;
   
    std::string to_string() const; 
};

std::ostream &operator<<(std::ostream &os, const Mino &b);

class OMino : public Mino {
    public:
    OMino(int x, int y, int orientation) : Mino(x, y) {
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
        
        
        for(int i = 0; i < orientation; i++) {
            InternalRotate(Rotation::Clockwise);
        }
        tile_type = Tile::O;
    }

    OMino(Board& board) : OMino(4, board.height-1, 0) {}

    Mino* Clone() override {
        return new OMino(x, y, orientation_);
    }

    Tile GetTile() override { return Tile::O; }
};

class LMino : public Mino {
    public:
    LMino(int x, int y, int orientation) : Mino(x, y) {
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
        
        
        for(int i = 0; i < orientation; i++) {
            InternalRotate(Rotation::Clockwise);
        }
        tile_type = Tile::L;
    }

    LMino(Board& board) : LMino(3, board.height-1, 0) {}
    
    Mino* Clone() override {
        return new LMino(x, y, orientation_);
    }
    
    Tile GetTile() override { return Tile::L; }
};

class JMino : public Mino {
    public:
    JMino(int x, int y, int orientation) : Mino(x, y) {
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
        
        
        for(int i = 0; i < orientation; i++) {
            InternalRotate(Rotation::Clockwise);
        }
        tile_type = Tile::J;
    }
    JMino(Board& board) : JMino(3, board.height-1, 0) {}
    
    Mino* Clone() override {
        return new JMino(x, y, orientation_);
    }
    
    Tile GetTile() override { return Tile::J; }
};

class SMino : public Mino {
    public:
    SMino(int x, int y, int orientation ) : Mino(x, y) {
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
        
        
        for(int i = 0; i < orientation; i++) {
            InternalRotate(Rotation::Clockwise);
        }
        tile_type = Tile::S;
    }
    SMino(Board& board) : SMino(3, board.height-1, 0) {}
    
    Mino* Clone() override {
        return new SMino(x, y, orientation_);
    }

    Tile GetTile() override { return Tile::S; }
};

class ZMino : public Mino {
    public:
    ZMino(int x, int y, int orientation) : Mino(x, y) {
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
        
        
        for(int i = 0; i < orientation; i++) {
            InternalRotate(Rotation::Clockwise);
        }
        tile_type = Tile::Z;
    }
    ZMino(Board& board) : ZMino(3, board.height-1, 0) {}

    Mino* Clone() override {
        return new ZMino(x, y, orientation_);
    }

    Tile GetTile() override { return Tile::Z; }
};

class IMino : public Mino {
    public:
    IMino(int x, int y, int orientation) : Mino(x, y) {
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
        
        
        for(int i = 0; i < orientation; i++) {
            InternalRotate(Rotation::Clockwise);
        }
        tile_type = Tile::I;
    }
    IMino(Board& board) : IMino(3, board.height-1, 0) {}
    
    Mino* Clone() override {
        return new IMino(x, y, orientation_);
    }

    Tile GetTile() override { return Tile::I; }
};

class TMino : public Mino {
    public:
    TMino(int x, int y, int orientation) : Mino(x, y) {
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

        
        for(int i = 0; i < orientation; i++) {
            InternalRotate(Rotation::Clockwise);
        }
        tile_type = Tile::T;
    }
    TMino(Board& board) : TMino(3, board.height-1, 0) {}
    
    Mino* Clone() override {
        return new TMino(x, y, orientation_);
    }
    
    Tile GetTile() override { return Tile::T; }
    
    virtual bool IsTSpin();
    virtual bool IsMiniTSpin();
};

} // namespace tetris

#endif
