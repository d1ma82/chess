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

    enum Choose {LONG_CASTLING, SHORT_CASTLING, MOVE};
    
    enum States {VOID, B_ROOK, B_KNIGHT, B_BISHOP, B_QUEEN, B_KING, B_PAWN,
                W_PAWN, W_ROOK, W_KHIGHT, W_BISHOP, W_QUEEN, W_KING};

    struct Move {
        States state;
        std::string move {'\0', '\0', '\0', '\0', '\0', '\0'};
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

    constexpr unsigned int selected_bit = 1<<8;
    constexpr unsigned int availabe_bit = 1<<9;
    constexpr unsigned int check_bit = 1<<11;
    int last_selected;      // last position
    unsigned int start_pos; // position from ray begins
    int king_pos, enemy_king;           
    std::vector<unsigned int> availables; // available moves
    std::vector<Move> moves;
    bool whites_;
    bool castling_enabled;
    bool choose_begin;
    bool wait=false;
    bool is_check, enemy_checked;
    on_move move_event;

    std::string state_to_str(States state) {

        switch (state) {
            case B_ROOK:   case W_ROOK:     return "ROOK";
            case B_KNIGHT: case W_KHIGHT:   return "KNIGHT";
            case B_BISHOP: case W_BISHOP:   return "BISHOP";
            case B_QUEEN:  case W_QUEEN:    return "QUEEN";
            case B_KING:   case W_KING:     return "KING";
            case B_PAWN:   case W_PAWN:     return "PAWN";
            default:       return "VOID";
        }
    }

    void init (bool whites, on_move listener) {
        
        move_event          = listener;
        last_selected       = 0;
        choose_begin        = false;
        whites_             = whites;
        castling_enabled    = true;
        is_check            = false;
        enemy_checked       = false;
        wait                = !whites;
        king_pos            = whites_? 60: 59;
        enemy_king          = whites_? 4: 3;
        availables.reserve(64);
        if (whites_) std::copy(init_position.begin(), init_position.end(), position.begin());
        else std::copy(init_position.rbegin(), init_position.rend(), position.begin());
    }

    inline bool empty(int pos) { return (position[pos]&0xFF) == VOID; }

    void add_available (int pos) {

        position[pos] += availabe_bit;
        availables.push_back(pos);
    }

    inline bool enemy(int at_pos) {
        return whites_? (position[at_pos]&0xFF) >= B_ROOK && (position[at_pos]&0xFF) <= B_PAWN
                      : (position[at_pos]&0xFF) >= W_PAWN && (position[at_pos]&0xFF) <= W_KING;
    }

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
    
    bool atacked (int);

    void next_empty (int at) {
        
        std::swap(position[at], position[start_pos]);
        bool king_atacked = atacked(king_pos);
        
        if (!is_check) { 
                // figure can only move when that not brings check
            if (king_atacked) { std::swap(position[at], position[start_pos]); return; }
            std::swap(position[at], position[start_pos]);
            add_available(at); 
            return; 
        }
        
        if (king_atacked) { std::swap(position[at], position[start_pos]); return; }
                     
        std::swap(position[at], position[start_pos]);   
        add_available(at);   
    }

    void non_empty (int at) {

        if (!enemy(at)) return;
        
        unsigned int old_state = position[at];
        position[at] = position[start_pos];
        position[start_pos] = VOID;
        bool king_atacked = atacked(king_pos);

        if (!is_check) { 
            
            if (king_atacked) { position[start_pos] = position[at]; position[at] = old_state; return; }
            position[start_pos] = position[at];
            position[at] = old_state;
            add_available(at); 
            return;
        }

        if (king_atacked) { position[start_pos] = position[at]; position[at] = old_state; return; }
        
        position[start_pos] = position[at];
        position[at] = old_state;
        add_available(at);        
    }

    bool pawn (int x, int y, int pos) {
        
        ray (3, x, y, next_empty, [] (int) {}, y==6? 2: 1);
        ray (2, x, y, [] (int) {}, non_empty, 1);
        ray (4, x, y, [] (int) {}, non_empty, 1);

        //TODO: intercept move
        return availables.size()>0;
    }

    bool rook (int x, int y) {
        
        for (int i=1; i<9; i+=2) ray(i, x, y, next_empty, non_empty); 
        return availables.size() > 0;
    }

    void calc_knight_pos(int x, int y, std::array<int, 8>& arr) {

        arr = { y-1<0 || x-2<0                      ? -1: (y-1)*BOARD_SIZE+x-2, 
                y-2<0 || x-1<0                      ? -1: (y-2)*BOARD_SIZE+x-1,
                y-2<0 || x+1>=BOARD_SIZE            ? -1: (y-2)*BOARD_SIZE+x+1,
                y-1<0 || x+2>=BOARD_SIZE            ? -1: (y-1)*BOARD_SIZE+x+2,
                y+1>=BOARD_SIZE || x+2>=BOARD_SIZE  ? -1: (y+1)*BOARD_SIZE+x+2,
                y+2>=BOARD_SIZE || x+1>=BOARD_SIZE  ? -1: (y+2)*BOARD_SIZE+x+1,
                y+2>=BOARD_SIZE || x-1<0            ? -1: (y+2)*BOARD_SIZE+x-1,
                y+1>=BOARD_SIZE || x-2<0            ? -1: (y+1)*BOARD_SIZE+x-2 };
    }

    bool knight (int x, int y) {

        std::array<int, 8> knight;        
        calc_knight_pos(x,  y, knight);

        std::for_each(knight.begin(), knight.end(), 
            [] (int v) { 
                if (v<0) return; 

                if (empty (v)) { next_empty (v); return; }
                non_empty (v);
        });
        return availables.size()>0;
    }

    bool bishop (int x, int y) {

        for (int i=2; i<9; i+=2) ray(i, x, y, next_empty, non_empty);
        return availables.size()>0;
    }

    bool queen (int x, int y) {

        for (int i=1; i<9; ++i) ray(i, x, y, next_empty, non_empty);
        return availables.size()>0;
    }

    bool atacked (int pos) {

        int x = pos%BOARD_SIZE, y = pos/BOARD_SIZE;
        std::array<int, 8> knight;
        calc_knight_pos(x,  y, knight);

        if (std::any_of(knight.begin(), knight.end(), 
            [] (int p) { return p>0 && (position[p]&0xFF) == (whites_? B_KNIGHT: W_KHIGHT); })) return true;

        for (int i=1; i<9; ++i) {

            bool ret = false;

            ray(i, x, y, [] (int) {}, 
                    [=, &ret] (int at) {
                        
                        switch (position[at]&0xFF) {
                        
                            case B_ROOK:    ret = whites_? i % 2 != 0: false; break; 
                            case B_KNIGHT:  ret = false; break;
                            case B_BISHOP:  ret = whites_? i % 2 == 0: false; break;
                            case B_QUEEN:   ret = whites_? true      : false; break;
                            case B_KING:    {int px=at%BOARD_SIZE, py=at/BOARD_SIZE; ret = whites_? abs(px-x)==1 || abs(y-py)==1: false; break;}
                            case B_PAWN:    {int px=at%BOARD_SIZE, py=at/BOARD_SIZE; ret = whites_? abs(px-x)==1 && y-py==1: false; break;}
                            case W_PAWN:    {int px=at%BOARD_SIZE, py=at/BOARD_SIZE; ret = whites_? false: abs(px-x)==1 && y-py==1; break;}
                            case W_ROOK:    ret = whites_? false     : i % 2 != 0; break; 
                            case W_KHIGHT:  ret = false; break;
                            case W_BISHOP:  ret = whites_? false     : i % 2 == 0; break; 
                            case W_QUEEN:   ret = whites_? false     : true; break; 
                            case W_KING:    {int px=at%BOARD_SIZE, py=at/BOARD_SIZE; ret = whites_? false: abs(px-x)==1 || abs(y-py)==1; break;}
                            default:        ret = false;
                        }                        
                    }
            ); 
            if (ret) return ret;       
        }          
        return false;
    }

    bool king (int x, int y, int pos) {

        for (int i=1; i<9; ++i) { 

            ray(i, x, y,
                [] (int pos) { if (!atacked(pos)) add_available(pos); }, 
                            [] (int pos) { if (enemy(pos) && !atacked(pos)) add_available(pos); }, 1);
        }        
        
        if (castling_enabled) {

            for (int i=1; i<6; i+=4) {

                bool free_way = true, atacked_pos = false, rook_moved = false;
                
                ray (i, x, y, 
                    [&] (int pos) { if (atacked(pos)) atacked_pos=true; }, 
                    
                    [&] (int pos) {
                        switch (position[pos]&0xFF) {
                            case W_ROOK: 
                                rook_moved = std::any_of(moves.begin(), moves.end(),
                                       [=] (const Move& m) { return std::strncmp((i==1? "a1": "h1"), m.move.c_str(), 2) == 0; });
                                break;
                            case B_ROOK:
                                rook_moved = std::any_of(moves.begin(), moves.end(),
                                        [=] (const Move& m) { return std::strncmp((i==1? "h8": "a8"), m.move.c_str(), 2) == 0; });
                                break; 
                            default: free_way = false;
                        }
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

    Choose make_long_castling (int where, int from) {
        
        if (whites_) {
            std::swap(position[where-2], position[where+1]);
        } else {
            std::swap(position[where+2], position[where-1]);
        }
        std::swap(position[where], position[from]);
        position[from] = VOID;
        return LONG_CASTLING;
    }

    Choose make_short_castling (int where, int from) {
        
        if (whites_) {
            std::swap(position[where+1], position[where-1]);
        } else {
            std::swap(position[where-1], position[where+1]);
        }
        std::swap(position[where], position[from]);
        position[from] = VOID;
        return SHORT_CASTLING;
    }

    Choose make_move(int where, int from) {
        
        if (empty (where)) { 
            std::swap(position[where], position[from]); 
            position[from] = VOID;          // clear any bits
        }
        else if (enemy(where)) { 
            position[where] = position[from]; 
            position[from] = VOID; 
        }        
        return MOVE;
    }

    Choose do_move (int where, int from) {
                
        if (castling_enabled && king_pos==from && abs(where-from)==2) {
             
             return whites_? from > where? make_long_castling(where, from): make_short_castling(where, from):
                from > where? make_short_castling(where, from): make_long_castling(where, from);

        } else {

           return make_move (where, from);
        }
    }

    void write_move (Choose choose, int from, int where) {


        switch (choose) {
            case LONG_CASTLING:  moves.emplace_back (States(position[where]&0xFF), "0-0-0"); break;
            case SHORT_CASTLING: moves.emplace_back (States(position[where]&0xFF), "0-0"); break;
            default: {
                    std::ostringstream str;
                    if (whites_) str << static_cast<char>('a'+from%BOARD_SIZE) << BOARD_SIZE-from/BOARD_SIZE << 
                            static_cast<char>('a'+where%BOARD_SIZE) << BOARD_SIZE-where/BOARD_SIZE;
                            
                    else str << static_cast<char>('h'-from%BOARD_SIZE) << 1+from/BOARD_SIZE <<
                                        static_cast<char>('h'-where%BOARD_SIZE) << 1+where/BOARD_SIZE;

                    moves.emplace_back (States(position[where]&0xFF), str.str());                        
            }
        }
    }

    void on_choose_end(int where, int from) {

        std::for_each(availables.begin(), availables.end(), 
                        [] (unsigned int v) { position[v] &= ~availabe_bit; });

        if (std::find(availables.begin(), availables.end(), where) == availables.end()) return;

        Choose choose = do_move (where, start_pos);
        write_move (choose, start_pos, where);

        if (king_pos==from) { king_pos = where; castling_enabled=false; }

        if (is_check) { is_check=false; position[king_pos] &= ~check_bit; }
        whites_=!whites_;
        if (atacked(enemy_king)) { enemy_checked=true; position[enemy_king] += check_bit; }
        whites_=!whites_;
        move_event({"move_done:"+moves.rbegin()->move}); 
        wait = !wait;       // wait for opponent

        LOGD("\t%s:\t%s", state_to_str(moves.rbegin()->state).c_str(), moves.rbegin()->move.c_str()) 
    }    
    /**
     * 
     * Callback from window, x and y converted from window coordinates to 0 to 7 coordinates
     * defines two ways, first for select figure and second for choosed cell to move
    */
    void on_select_cell (int x, int y) {
        
        if (wait) return;
        position[last_selected] &= ~selected_bit;

        int choosed = y*BOARD_SIZE+x; 

        if (choose_begin) {
            
            on_choose_end(choosed, start_pos);
            choose_begin    = false;
            last_selected   = 0;
            availables.clear();

        } else {
                // Begin construct availabe moves 
            start_pos = choosed;
            choose_begin = on_choose_begin (States(position[choosed]&0xFF), x, y, choosed);
            position[choosed] += selected_bit; 
            last_selected = choosed;
        }
    }

/**
 * Retrieve opponent move in chess notation, convert to local coordinates, apply move
 * 
*/
    void opponent_move (std::string_view move) {

        int from=0, where=0;
        bool old_castling;
        int old_king_pos;
        
        if (move.compare("0-0") == 0) {

            old_castling = castling_enabled, old_king_pos = king_pos, king_pos=enemy_king;
            castling_enabled=true;

            if (whites_) { from = 4, where = 6; } else { from = 3, where = 1; }
            do_move (where, from);
            enemy_king = where, castling_enabled=old_castling, king_pos=old_king_pos;
        }
        else if (move.compare("0-0-0") == 0) {

            old_castling = castling_enabled, old_king_pos = king_pos, king_pos=enemy_king;
            castling_enabled=true;

            if (whites_) { from = 4, where = 2; } else { from = 3, where = 5; }
            do_move(where, from);
            enemy_king = where, castling_enabled=old_castling, king_pos=old_king_pos;

        } else {
            if (whites_) {
                from  = ('8'-move[1]) * BOARD_SIZE + (move[0]-'a');
                where = ('8'-move[3]) * BOARD_SIZE + (move[2]-'a');
            } else {
                from  = (move[1]-'1') * BOARD_SIZE + ('h'-move[0]);
                where = (move[3]-'1') * BOARD_SIZE + ('h'-move[2]);
            }
            whites_=!whites_;
            do_move (where, from);
            whites_=!whites_;
            if (from==enemy_king) enemy_king=where;
        }
        if (enemy_checked) { enemy_checked=false; position[enemy_king] &= ~check_bit; }
        moves.emplace_back (States(position[where]&0xFF), std::string(move));
        is_check = atacked(king_pos);
        if (is_check) position[king_pos]+=check_bit;
        
        wait = !wait;
        LOGD("Opponent: \t%s:\t%s", state_to_str(moves.rbegin()->state).c_str(), moves.rbegin()->move.c_str()) 
    }

    void clear() {

    }
}