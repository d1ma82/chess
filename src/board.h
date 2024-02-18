#pragma once
#include "opengl.h"
#include "filter.h"
#include "types.h"
#include <vector>

namespace filter
{   
    const int FIGURE_COUNT = 12;

    class Board final : public filter::OpenGL {
    private:
        GLuint VAO = 0, VBO = 0, IBO=0;
        GLuint VS=0, FS=0, PR=0;

        GLuint texture_array = 0;

        GLfloat* vertices {nullptr};
        GLuint v_size = 0;
        GLuint* indices {nullptr};
        GLuint i_size = 0;

        GLint err;
        GLint position_location;

        unsigned int board_size_;
        const unsigned int cells;
        GLuint* position {nullptr};

        void init_buffers (); 
        void init_gl_buffers();
        void load_textures(dims view);
    public:
        Board (unsigned int board_size, dims view);
        ~Board();
        
        Board (const Board& other) = delete;
        Board (Board&& other) = delete;
        Board& operator = (const Board& other) = delete;
        Board& operator = (Board&& other) = delete;

        void apply ();
        void set_uniformuiv (unsigned int* data) { position=data; }
    };

    class Arrow final: public filter::OpenGL {
    private:
        GLuint VAO = 0, VBO = 0;
        GLuint VS=0, FS=0, PR=0;
        size_t buff_size;
        GLfloat* arrow;
        GLint err;
         
        void init_gl_buffers();
    public:
        Arrow(float* buffer, size_t size);
        ~Arrow();
        void apply();
    };
}
