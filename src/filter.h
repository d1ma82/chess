#pragma once

namespace filter {
    
    class Base {
    protected:
        Base () {};
    public:
        virtual ~Base () = default;
        virtual void apply()=0;
    };

    class OpenGL: public Base {
    public:
        virtual void set_uniformuiv(unsigned int*)=0;
    };
}