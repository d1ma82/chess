#include "chess.h"
#include <set>
#include <vector>
#include <cstring>
#include <sstream>
#include <algorithm>
#include <functional>
#include <cassert>
#include "log.h"

namespace chess {

    using on_progress = std::function<void (int)>;
    using on_non_empty = std::function<void (int)>;

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
    std::array<int, 8> knight_pos;  
    bool whites_;
    bool castling_enabled;
    bool king_select;
    bool wait=false;
    on_move move_event;

    std::string state_to_str(States state) {

        assert(state!=VOID);
        switch (state) {
            case B_ROOK:   case W_ROOK:     return "ROOK";
            case B_KNIGHT: case W_KHIGHT:   return "KNIGHT";
            case B_BISHOP: case W_BISHOP:   return "BISHOP";
            case B_QUEEN:  case W_QUEEN:    return "QUENN";
            case B_KING:   case W_KING:     return "KING";
            case B_PAWN:   case W_PAWN:     return "PAWN";
            default:       return "VOID";
        }
    }

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

    inline void add_available (int pos) {

        position[pos] += move_bit;
        availables.push_back(pos);
    }

    inline bool enemy(int at_pos) {
        return whites_? position[at_pos] > 0 && position[at_pos] < 7
                      : position[at_pos] > 6 && position[at_pos] < 13;
    }

    inline bool empty(int pos) { return (position[pos]&0xFF) == VOID; }

    template<int dx, int dy>
    void go (int x, int y, int lim, on_progress progress, on_non_empty non_empty) {

        if (x<0 || y==BOARD_SIZE || x==BOARD_SIZE || y<0 || lim==0) return;
        
        int pos = y*BOARD_SIZE+x;
        if (empty(pos)) progress(pos); else { non_empty(pos); return; }
        go<dx, dy>(x+dx, y+dy, lim-1, progress, non_empty);
    }

    void ray(int direction, int from_x, int from_y, on_progress progress, on_non_empty non_empty, int lim=-1) {

        switch (direction) {
            case 1:     go<-1, 0> (from_x-1, from_y,   lim, progress, non_empty); break;            //run_west                 
            case 2:     go<-1, -1>(from_x-1, from_y-1, lim, progress, non_empty); break;            //run_north_west                                             
            case 3:     go<0, -1> (from_x,   from_y-1, lim, progress, non_empty); break;            //run_north             
            case 4:     go<1, -1> (from_x+1, from_y-1, lim, progress, non_empty); break;            //run_north_east 
            case 5:     go<1, 0>  (from_x+1, from_y,   lim, progress, non_empty); break;            //run_east       
            case 6:     go<1, 1>  (from_x+1, from_y+1, lim, progress, non_empty); break;            //run_south_east 
            case 7:     go<0, 1>  (from_x,   from_y+1, lim, progress, non_empty); break;            //run_south      
            case 8:     go<-1, 1> (from_x-1, from_y+1, lim, progress, non_empty); break;            //run_south_west 
        }
    }

    bool pawn (int x, int y, int pos) {
        
        ray (3, x, y, add_available, [] (int) {}, y==6? 2: 1);
        ray (2, x, y, [] (int) {}, [] (int pos) { if (enemy(pos)) add_available(pos); }, 1);
        ray (4, x, y, [] (int) {}, [] (int pos) { if (enemy(pos)) add_available(pos); }, 1);

        //TODO: intercept move
        return availables.size()>0;
    }

    bool rook (int x, int y) {
        
        for (int i=1; i<9; i+=2) ray(i, x, y, add_available, [] (int pos) { if (enemy(pos)) add_available(pos); });
        return availables.size() > 0;
    }

    void calc_knight_pos(int x, int y) {

        knight_pos = {
            y-1<0 || x-2<0                      ? -1: (y-1)*BOARD_SIZE+x-2, 
            y-2<0 || x-1<0                      ? -1: (y-2)*BOARD_SIZE+x-1,
            y-2<0 || x+1>=BOARD_SIZE            ? -1: (y-2)*BOARD_SIZE+x+1,
            y-1<0 || x+2>=BOARD_SIZE            ? -1: (y-1)*BOARD_SIZE+x+2,
            y+1>=BOARD_SIZE || x+2>=BOARD_SIZE  ? -1: (y+1)*BOARD_SIZE+x+2,
            y+2>=BOARD_SIZE || x+1>=BOARD_SIZE  ? -1: (y+2)*BOARD_SIZE+x+1,
            y+2>=BOARD_SIZE || x-1<0            ? -1: (y+2)*BOARD_SIZE+x-1,
            y+1>=BOARD_SIZE || x-2<0            ? -1: (y+1)*BOARD_SIZE+x-2
        };
    }

    bool knight (int x, int y) {
                    
        calc_knight_pos(x,  y);

        std::for_each(knight_pos.begin(), knight_pos.end(), 
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

        for (int i=2; i<9; i+=2) ray(i, x, y, add_available, [] (int pos) { if (enemy(pos)) add_available(pos); });
        return availables.size()>0;
    }

    bool queen (int x, int y) {

        for (int i=1; i<9; ++i) ray(i, x, y, add_available, [] (int pos) { if (enemy(pos)) add_available(pos); });
        return availables.size()>0;
    }

    bool atacked (int pos) {

        int x = pos%BOARD_SIZE, y = pos/BOARD_SIZE;
        for (int i=1; i<9; ++i) {

            bool value;
            ray(i, x, y, [] (int) {}, 
                    [=, &value] (int at) {
                        switch (position[at]&0xFF) {
                        
                            case B_ROOK:    value = whites_? i % 2 != 0: false; break; 
                            case B_BISHOP:  value = whites_? i % 2 == 0: false; break;
                            case B_QUEEN:   value = whites_? true      : false; break;
                            case B_KING:    value = whites_? true : false; break;
                            case B_PAWN:    {int px=at/BOARD_SIZE, py=at%BOARD_SIZE; 
                                             LOGD("%d, %d, %d, %d, %d", px, py, x, y, i) 
                                             value = whites_? abs(px-x)==1 && y-py==1: false; break;}
                     //       case W_PAWN:    value = whites_? false     : (i==2 && length==2) || (i==4 && length==2); break;
                            case W_ROOK:    value = whites_? false     : i % 2 != 0; break; 
                            case W_BISHOP:  value = whites_? false     : i % 2 == 0; break; 
                            case W_QUEEN:   value = whites_? false     : true; break; 
                            case W_KING:    value = whites_? false     : true; break;
                            default:        value = false;
                        }                        
                    }
            ); 
            if (value) return true;        
        }          
        return false;
    }

    bool king (int x, int y, int pos) {

        king_select=true;
        for (int i=1; i<9; ++i) { 

            ray(i, x, y,
                [] (int pos) { if (!atacked(pos)) add_available(pos); }, 
                            [] (int pos) { if (enemy(pos)) add_available(pos); }, 1);
        }        
        
        if (castling_enabled) {

            for (int i=1; i<6; i+=4) {

                bool free_way = true, atacked_pos = false, rook_moved = false;
                
                ray (i, x, y, 
                    [&] (int pos) { if (atacked(pos)) atacked_pos=true; }, 
                    
                    [&] (int pos) { 
                        if (position[pos] == W_ROOK) { 
                                rook_moved = std::any_of(moves.begin(), moves.end(),
                                        [=] (const Move& m) { return std::strncmp((i==1? "a1": "h1"), m.half.c_str(), 2) == 0; });
                                return;
                        } else if (position[pos] == B_ROOK) {
                                rook_moved = std::any_of(moves.begin(), moves.end(),
                                        [=] (const Move& m) { return std::strncmp((i==1? "h8": "a8"), m.half.c_str(), 2) == 0; });
                                return;                                
                        }
                        free_way = false;
                    }
                );
                if (free_way && !rook_moved && !atacked_pos) add_available(i==1? pos-2: pos+2);
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

        if (king_select) castling_enabled=false;
        
        if (whites_) str << static_cast<char>('a'+from%BOARD_SIZE) << BOARD_SIZE-from/BOARD_SIZE << 
                                    static_cast<char>('a'+where%BOARD_SIZE) << BOARD_SIZE-where/BOARD_SIZE;
                                    
        else str << static_cast<char>('h'-from%BOARD_SIZE) << 1+from/BOARD_SIZE <<
                            static_cast<char>('h'-where%BOARD_SIZE) << 1+where/BOARD_SIZE;
        return str.str();         
    }

    void on_choose_end (int where, int from) {
                
        if (castling_enabled && king_select && abs(where-from)==2) {
             
             if (whites_) {
                moves.emplace_back (States(position[where]&0xFF), 
                    (from > where? make_long_castling(where, from): make_short_castling(where, from)));
             } else {
                moves.emplace_back (States(position[where]&0xFF), 
                    (from > where? make_short_castling(where, from): make_long_castling(where, from)));
             }
        } else {
            moves.emplace_back (States(position[where]&0xFF), make_move (where, from));
        }
        LOGD("\t%s:\t%s", state_to_str(States(position[where]&0xFF)).c_str(), moves.rbegin()->half.c_str()) 
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
            choosed_pos = on_choose_begin (States(position[pos]&0xFF), x, y, pos)? pos: -1;
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