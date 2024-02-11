#include "chess.h"
#include <vector>
#include <sstream>
#include <algorithm>
#include "log.h"

namespace chess {

   enum States {VOID, B_ROOK, B_KNIGHT, B_BISHOP, B_QUEEN, B_KING, B_PAWN,
                W_PAWN, W_ROOK, W_KHIGHT, W_BISHOP, W_QUEEN, W_KING};

    struct Move {
        States state;
        std::string half {'\0', '\0', '\0', '\0'};
    };

   const std::array<unsigned int, BOARD_SIZE*BOARD_SIZE> init_position =  {
        1,  2,  3,  4,  5,  3,  2,  1,
        6,  6,  6,  6,  6,  6,  6,  6,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        7,  7,  7,  7,  7,  7,  7,  7,
        8,  9,  10, 11, 12, 10, 9,  8
    };

    const unsigned int selected_bit = 1<<8;
    const unsigned int move_bit = 1<<9;
    int last_selected;      // last position
    int choosed_pos;        // choosed position
    std::vector<unsigned int> availables; // available moves
    std::vector<Move> moves;
    bool whites_;
    bool castling_enabled;
    bool king_select;
    bool wait=false;
    on_move move_event;

    void init (bool whites, on_move listener) {
        
        move_event          = listener;
        last_selected       =-1;
        choosed_pos         =-1;
        whites_             = whites;
        castling_enabled    = true;
        king_select         = false;
        wait                = !whites;
        availables.reserve(64);
        if (whites_) std::copy(init_position.begin(), init_position.end(), position.begin());
        else std::copy(init_position.rbegin(), init_position.rend(), position.begin());
    }

    void add_available (int pos) {

        position[pos] += move_bit;
        availables.push_back(pos);
    }

    bool enemy(int at_pos) {
        return whites_? position[at_pos] > 0 && position[at_pos] < 7
                      : position[at_pos] > 6 && position[at_pos] < 13;
    }

    inline bool empty(int pos) { return (position[pos]&0xFF) == VOID; }

    bool pawn (int x, int y, int pos) {

        //TODO: intercept move
        int stride=pos-BOARD_SIZE;

        if (empty (stride)) {
            
            add_available (stride);
            if (y==6) {     // First move
                stride -= BOARD_SIZE;
                if (empty(stride)) add_available (stride);
            }
        }

        if (x>0) {
            
            stride = pos-(BOARD_SIZE+1);
            if (!empty (stride) && enemy (stride)) add_available (stride);
        } 
        if (x+1 < BOARD_SIZE) {
            
            stride = pos-(BOARD_SIZE-1);
            if (!empty (stride) && enemy (stride)) add_available (stride);
        }

        return availables.size()>0;
    }

    void run_west (int from_x, int from_y, int lim) {

        if (from_x<0 || lim==0) return;
        int pos = from_y*BOARD_SIZE+from_x;

        if (!empty (pos)) {

            if (enemy(pos)) add_available (pos);
            return;
        }
        add_available (pos); 
        run_west (--from_x, from_y, --lim);
    }

    void run_north (int from_x, int from_y, int lim) {

        if (from_y<0 || lim==0) return;
        int pos = from_y*BOARD_SIZE+from_x;

        if (!empty (pos)) {

            if (enemy(pos)) add_available (pos);
            return;
        }
        add_available (pos);
        run_north(from_x, --from_y, --lim);
    }

    void run_east (int from_x, int from_y, int lim) {

        if (from_x==BOARD_SIZE || lim==0) return;
        int pos = from_y*BOARD_SIZE+from_x;

        if (!empty (pos)) {

            if (enemy(pos)) add_available (pos);
            return;
        }
        add_available (pos);
        run_east(++from_x, from_y, --lim);        
    }

    void run_south (int from_x, int from_y, int lim=-1) {

        if (from_y==BOARD_SIZE || lim==0) return;
        int pos = from_y*BOARD_SIZE+from_x;

        if (!empty (pos)) {

            if (enemy(pos)) add_available (pos);
            return;
        }
        add_available (pos);
        run_south (from_x, ++from_y, --lim);
    }

    void run_north_west (int from_x, int from_y, int lim) {

        if (from_x<0 || from_y<0 || lim==0) return;
        int pos = from_y*BOARD_SIZE+from_x;

        if (!empty (pos)) {

            if (enemy(pos)) add_available (pos);
            return;
        }
        add_available (pos);
        run_north_west (--from_x, --from_y, --lim);
    }

    void run_north_east (int from_x, int from_y, int lim) {

        if (from_x==BOARD_SIZE || from_y<0 || lim==0) return;
        int pos = from_y*BOARD_SIZE+from_x;

        if (!empty (pos)) {

            if (enemy(pos)) add_available (pos);
            return;
        }
        add_available (pos);
        run_north_east (++from_x, --from_y, --lim);        
    }

    void run_south_east (int from_x, int from_y, int lim) {

        if (from_x==BOARD_SIZE || from_y==BOARD_SIZE || lim==0) return;
        int pos = from_y*BOARD_SIZE+from_x;

        if (!empty (pos)) {

            if (enemy(pos)) add_available (pos);
            return;
        }
        add_available (pos);
        run_south_east (++from_x, ++from_y, --lim);          
    }

    void run_south_west (int from_x, int from_y, int lim) {

        if (from_x<0 || from_y==BOARD_SIZE || lim==0) return;
        int pos = from_y*BOARD_SIZE+from_x;

        if (!empty (pos)) {

            if (enemy(pos)) add_available (pos);
            return;
        }
        add_available (pos);
        run_south_west (--from_x, ++from_y, --lim);         
    }

    void ray(int direction, int from_x, int from_y, int lim=-1) {

        switch (direction) {
            case 1:     run_west       (--from_x, from_y, lim);   break;          
            case 2:     run_north_west (--from_x, --from_y, lim); break;                                            
            case 3:     run_north      (from_x, --from_y, lim);   break;       
            case 4:     run_north_east (++from_x, --from_y, lim); break;
            case 5:     run_east       (++from_x, from_y, lim);   break;
            case 6:     run_south_east (++from_x, ++from_y, lim); break;
            case 7:     run_south      (from_x, ++from_y, lim);   break;
            case 8:     run_south_west (--from_x, ++from_y, lim); break;
        }
    }

    bool rook (int x, int y) {
        
        for (int i=1; i<9; i+=2) ray(i, x, y);
        return availables.size() > 0;
    }

    bool knight (int x, int y) {
                    
        std::array<int, 8> pos {
            y-1<0 || x-2<0                      ? -1: (y-1)*BOARD_SIZE+x-2, 
            y-2<0 || x-1<0                      ? -1: (y-2)*BOARD_SIZE+x-1,
            y-2<0 || x+1>=BOARD_SIZE            ? -1: (y-2)*BOARD_SIZE+x+1,
            y-1<0 || x+2>=BOARD_SIZE            ? -1: (y-1)*BOARD_SIZE+x+2,
            y+1>=BOARD_SIZE || x+2>=BOARD_SIZE  ? -1: (y+1)*BOARD_SIZE+x+2,
            y+2>=BOARD_SIZE || x+1>=BOARD_SIZE  ? -1: (y+2)*BOARD_SIZE+x+1,
            y+2>=BOARD_SIZE || x-1<0            ? -1: (y+2)*BOARD_SIZE+x-1,
            y+1>=BOARD_SIZE || x-2<0            ? -1: (y+1)*BOARD_SIZE+x-2
        };

        std::for_each(pos.begin(), pos.end(), 
            [] (int v) { 
                if (v>=0) {

                    if (!empty (v)) {

                        if (enemy(v)) add_available (v);
                        return;
                    }
                    add_available (v);
                }
        });
        return availables.size()>0;
    }

    bool bishop (int x, int y) {

        for (int i=2; i<9; i+=2) ray(i, x, y);
        return availables.size()>0;
    }

    bool queen (int x, int y) {

        for (int i=1; i<9; ++i) ray(i, x, y);
        return availables.size()>0;
    }

    bool king (int x, int y, int pos) {

        king_select=true;

        for (int i=1; i<9; ++i) ray(i, x, y, 1);

        if (castling_enabled) {

            auto itw = moves.end(), itb = moves.end();
            bool rook_moved=false, free_way=true;

            for (int i=56; i<BOARD_SIZE*BOARD_SIZE; i++) {

                switch (position[i]&0xFF) {
                    case W_ROOK:
                        itw = std::find_if(moves.begin(), moves.end(),
                            [=] (const Move& m) { return (i==56? "a1":"h1") == m.half.substr(2); });
                        rook_moved = itw != moves.end();
                        if (!rook_moved && free_way && i==BOARD_SIZE*BOARD_SIZE-1) add_available(i-1);
                        break;
                    case W_KING:
                        if (!rook_moved && free_way) add_available(i-2);
                        rook_moved=false;
                        free_way=true;
                        break;
                    case B_ROOK:
                        itb = std::find_if(moves.begin(), moves.end(), 
                            [=] (const Move& m) { return (i==56? "h8":"a8") == m.half.substr(2); });
                        rook_moved = itb != moves.end();
                        if (!rook_moved && free_way && i==BOARD_SIZE*BOARD_SIZE-1) add_available(i-2);                                  
                        break;
                    case B_KING:
                        if (!rook_moved && free_way) add_available(i-2);
                        rook_moved=false;
                        free_way=true;
                        break;
                    default: if (!empty(i) && free_way) free_way=false;
                }
            }
        }
        return availables.size()>0;
    }

    bool on_choose_begin (States state, int x, int y, int pos) {

        switch (state&0xFF) {

            case B_ROOK:    return whites_? false           : rook(x, y);
            case B_KNIGHT:  return whites_? false           : knight(x, y);
            case B_BISHOP:  return whites_? false           : bishop(x, y);
            case B_QUEEN:   return whites_? false           : queen(x, y);
            case B_KING:    return whites_? false           : king(x, y, pos);
            case B_PAWN:    return whites_? false           : pawn(x,y,pos);
            case W_PAWN:    return whites_? pawn(x,y,pos)   : false;
            case W_ROOK:    return whites_? rook(x, y)      : false;
            case W_KHIGHT:  return whites_? knight(x, y)    : false;
            case W_BISHOP:  return whites_? bishop(x, y)    : false;
            case W_QUEEN:   return whites_? queen(x, y)     : false;
            case W_KING:    return whites_? king(x, y, pos) : false;
            default:        return false;
        }
    }

    std::string make_long_castling (int where, int from) {
        
        LOGD("Long %d:%d", from, where)
        if (whites_) {
            std::swap(position[where-2], position[where+1]);
        } else {
            std::swap(position[where+2], position[where-1]);
        }
        std::swap(position[where], position[from]);
        castling_enabled=false;
        return "0-0-0";
    }

    std::string make_short_castling (int where, int from) {
        
        LOGD("Short %d:%d", from, where)
        if (whites_) {
            std::swap(position[where+1], position[where-1]);
        } else {
            std::swap(position[where-1], position[where+1]);
        }
        std::swap(position[where], position[from]);
        castling_enabled=false;
        return "0-0";
    }

    std::string make_move(int where, int from) {

        std::ostringstream str;
        if (empty (where)) std::swap(position[where], position[from]);
        else if (enemy(where)) { position[where] = position[from]; position[from] = VOID; }
        
        if (whites_) str << static_cast<char>('a'+from%BOARD_SIZE) << BOARD_SIZE-from/BOARD_SIZE << 
                                    static_cast<char>('a'+where%BOARD_SIZE) << BOARD_SIZE-where/BOARD_SIZE;
                                    
        else str << static_cast<char>('h'-from%BOARD_SIZE) << 1+from/BOARD_SIZE <<
                            static_cast<char>('h'-where%BOARD_SIZE) << 1+where/BOARD_SIZE;
        return str.str();         
    }

    void on_choose_end (int where, int from) {
                
        if (castling_enabled && king_select && abs(where-from)==2) {
             
             if (whites_) {
                moves.emplace_back (States(position[where]), 
                    (from > where? make_long_castling(where, from): make_short_castling(where, from)));
             } else {
                moves.emplace_back (States(position[where]), 
                    (from > where? make_short_castling(where, from): make_long_castling(where, from)));
             }
        } else {
            moves.emplace_back (States(position[where]), make_move (where, from));
        } 
    }
    /**
     * 
     * Callback from window, x and y converted from window coordinates to 0 to 7 coordinates
     * defines two ways, first for select figure and second for choosed cell to move
    */
    void on_select_cell (int x, int y) {
        
        if (wait) return;

        if (last_selected>=0) position[last_selected] &= ~selected_bit; //unset select
        
        int pos = y*BOARD_SIZE+x;        
        
        if (choosed_pos>=0) {
            
            std::for_each(availables.begin(), availables.end(), 
                        [] (unsigned int v) { position[v] &= ~move_bit; });             // remove move bit
            
            if (std::find(availables.begin(), availables.end(), pos) != availables.end()) {

                on_choose_end (pos, choosed_pos);
                move_event("move_done:"+moves.rbegin()->half); 
                wait = !wait;       // wait for opponent
            }
            pos=-1;
            king_select=false;
            choosed_pos = -1;
            availables.clear();

        } else {
                // Begin construct availabe moves
            choosed_pos = on_choose_begin (States(position[pos]), x, y, pos)? pos: -1;
            position[pos] += selected_bit; 
        }
        last_selected = pos;
    }
/**
 * Retrieve opponent move in chess notation, convert to local coordinates, apply move
 * 
*/
    void opponent_move (const std::string& move) {

        LOGD("Op move %s", move.c_str())
        int from=0, where=0;
        bool old_castling, old_king;

        if (move.compare("0-0") == 0) {

            old_castling = castling_enabled, old_king=king_select;
            castling_enabled=true, king_select=true;

            if (whites_) { from = 4, where = 6; } else { from = 3, where = 1; }
            on_choose_end(where, from);
            castling_enabled=old_castling, king_select=old_king;
        }
        else if (move.compare("0-0-0") == 0) {

            old_castling = castling_enabled, old_king=king_select;
            castling_enabled=true, king_select=true;
            if (whites_) { from = 4, where = 2; } else { from = 3, where = 5; }
            on_choose_end(where, from);
            castling_enabled=old_castling, king_select=old_king;

        } else {
            if (whites_) {
                from  = ('8'-move[1]) * BOARD_SIZE + (move[0]-'a');
                where = ('8'-move[3]) * BOARD_SIZE + (move[2]-'a');
            } else {
                from  = (move[1]-'1') * BOARD_SIZE + ('h'-move[0]);
                where = (move[3]-'1') * BOARD_SIZE + ('h'-move[2]);
            }
            whites_=!whites_;
            on_choose_end (where, from);
            whites_=!whites_;
            LOGD("Parsed move %d:%d", from, where)
        }
        wait = !wait;
    }

    void clear() {

    }
}