add_library(libtetris core/Tetris.cpp util/Fumen.cpp)
add_library(bot bot/MoveGen.cpp bot/Analysis.cpp core/Tetris.cpp third_party/uint256_t.cpp third_party/uint128_t.cpp)
