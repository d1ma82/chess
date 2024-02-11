#pragma once
#include <functional>
#include "filter.h"

namespace window {

    const int PRESS = 1;

    const int KEY_RIGHT = 262;
    const int KEY_LEFT  = 263;
    const int KEY_UP    = 265;
    const int KEY_DOWN  = 264;
    const int KEY_ESCAPE = 256;

    using key_callback = std::function<void(int key, int action)>;
    using mouse_click_callback = std::function<void (double X, double Y)>;

    
    class Window {
    protected:
        Window() = default;
    public:
        virtual bool should_close()=0;
        virtual void close()=0;
        virtual void draw()=0;
        virtual void set_key_event_listener(key_callback listener)=0;
        virtual void set_mouse_key_listener(mouse_click_callback listener)=0;
        virtual ~Window()=default;
    };
}