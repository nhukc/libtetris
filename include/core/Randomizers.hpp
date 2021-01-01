#ifndef RANDOMIZERS_HPP
#define RANDOMIZERS_HPP

#include "../core/Tetris.hpp"
#include <memory>

namespace tetris {

class Randomizer {
    public:
    virtual std::unique_ptr<tetris::Mino> GetNextMino() = 0;
};

class SevenBagRandomizers : public Randomizer {
    protected:
    std::vector<tetris::Mino> current_bag;
};

class ClassicRandomizer : public Randomizer {
    
};

class FiniteSequenceRandomizer : public Randomizer {
    protected:
    std::vector<tetris::Mino> mino_sequence;
};

}

#endif
