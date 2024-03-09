#pragma once
#include <array>
#include <string>
#include <functional>

namespace chess {
    
    constexpr int BOARD_SIZE=8;
    // When read state use &0xFF cause for state used 1 byte, other 3 bytes used for flags
    inline std::array<unsigned int, BOARD_SIZE*BOARD_SIZE> position; 
    using on_move = std::function<void (std::string_view move)>;
    using on_move_coord = std::function<void (unsigned int from, unsigned int to)>;

    void init(bool whites, on_move listener, on_move_coord opponent_move_listener);
    void on_select_cell (int x, int y);
    void opponent_move (std::string_view move);
    void clear();
}