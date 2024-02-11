#pragma once
#include <array>
#include <string>
#include <functional>

namespace chess {
    
    inline const int BOARD_SIZE=8;
    inline std::array<unsigned int, BOARD_SIZE*BOARD_SIZE> position;
    using on_move = std::function<void (const std::string& move)>;

    void init(bool whites, on_move listener);
    void on_select_cell (int x, int y);
    void opponent_move (const std::string& move);
    void clear();
}