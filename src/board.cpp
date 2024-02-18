#include "board.h"
#include "log.h"
#include <array>
#include <sstream>
#include <algorithm>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace filter {

    void Board::init_buffers () {

        v_size = sizeof(GLfloat)*40*cells;
        i_size = sizeof(GLuint)*6*cells;

        vertices = new GLfloat[v_size];
        indices = new GLuint[i_size];

        int id_v = -1, id_i=-1;
        std::array<GLuint, 6> indices_index {0,1,3,1,2,3};
        const std::array<float, 3> color1 {0.0f, 1.0f, 1.0f};
        const std::array<float, 3> color2 {1.0f, 1.0f, 0.0f};
        
        float size = 2.0f/ static_cast<float>(board_size_);          // size of cell

        for (unsigned int i=0; i<board_size_; ++i) {
            for (unsigned int j=0; j<board_size_; ++j) {
                    
                float x = -1.0f + j * size;     // left
                float y = 1.0f - i * size;      // top
             
                // Color
                const std::array<float, 3>& color = ((i + j) % 2 == 0) ? color1 : color2;
            
                //Vertices

                vertices[++id_v] = x+size, vertices[++id_v] = y, vertices[++id_v] = 0.0f,                   // top right
                vertices[++id_v] = color[0], vertices[++id_v] = color[1], vertices[++id_v] = color[2], 
                vertices[++id_v] = 1.0f, vertices[++id_v] = 0.0f,
                vertices[++id_v] = static_cast<float>(j), vertices[++id_v] = static_cast<float>(i);                                   // chessboard coord                      
                
                vertices[++id_v] = x+size, vertices[++id_v] = y-size, vertices[++id_v] = 0.0f,              // bottom right 
                vertices[++id_v] = color[0], vertices[++id_v] = color[1], vertices[++id_v] = color[2],    
                vertices[++id_v] = 1.0f, vertices[++id_v] = 1.0f,
                vertices[++id_v] = static_cast<float>(j), vertices[++id_v] = static_cast<float>(i);                        
                
                vertices[++id_v] = x, vertices[++id_v] = y-size, vertices[++id_v] = 0.0f,                   // bottom left
                vertices[++id_v] = color[0], vertices[++id_v] = color[1], vertices[++id_v] = color[2],    
                vertices[++id_v] = 0.0f, vertices[++id_v] = 1.0f,
                vertices[++id_v] = static_cast<float>(j), vertices[++id_v] = static_cast<float>(i); 
                
                vertices[++id_v] = x, vertices[++id_v] = y, vertices[++id_v] = 0.0f,                        // top left    
                vertices[++id_v] = color[0], vertices[++id_v] = color[1], vertices[++id_v] = color[2],                
                vertices[++id_v] = 0.0f, vertices[++id_v] = 0.0f,
                vertices[++id_v] = static_cast<float>(j), vertices[++id_v] = static_cast<float>(i); 

                for (auto& el: indices_index) { indices[++id_i] = el; el+=4; }
            }
        }      
    }

    void Board::init_gl_buffers () {

        VS = gl::create_shader_file ("../shader/board.vert", GL_VERTEX_SHADER);
        FS = gl::create_shader_file ("../shader/board.frag", GL_FRAGMENT_SHADER);

        PR = gl::create_programm(VS, FS);

        CALLGL(glGenVertexArrays(1, &VAO))
        CALLGL(glGenBuffers(1, &VBO))
        CALLGL(glGenBuffers(1, &IBO))

        CALLGL(glBindVertexArray(VAO))

        CALLGL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO))
        CALLGL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, i_size, indices, GL_STATIC_DRAW))

        CALLGL(glBindBuffer(GL_ARRAY_BUFFER, VBO))
        CALLGL(glBufferData(GL_ARRAY_BUFFER, v_size, vertices, GL_STATIC_DRAW))

        CALLGL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 10*sizeof(GL_FLOAT), (void*)0))
        CALLGL(glEnableVertexAttribArray(0))

        CALLGL(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 10*sizeof(GL_FLOAT), (void*)(3*sizeof(GL_FLOAT))))
        CALLGL(glEnableVertexAttribArray(1))

        CALLGL(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 10*sizeof(GL_FLOAT), (void*)(6*sizeof(GL_FLOAT))))
        CALLGL(glEnableVertexAttribArray(2))

        CALLGL(glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 10*sizeof(GL_FLOAT), (void*)(8*sizeof(GL_FLOAT))))
        CALLGL(glEnableVertexAttribArray(3))

        CALLGL(glBindVertexArray(0))
    }

    void Board::load_textures(dims view) {

        int width, height, channels = 0;
        CALLGL(glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &texture_array))
        CALLGL(glTextureStorage3D(texture_array, 1, GL_RGBA8, view.first/board_size_, view.first/board_size_, FIGURE_COUNT))
       
        for (int i=0; i<FIGURE_COUNT; ++i) {
            
            std::ostringstream path;
            path << "../png/" << i+1 << ".png";
            unsigned char* image = stbi_load(path.str().c_str(), &width, &height, &channels, 0);
            
            if (image==nullptr) {

                LOGI("Could not load image %s", path.str().c_str())
                continue;
            } else {
               // LOGI("%s %dx%dx%d", path.str().c_str(), width, height, channels)
            }
            CALLGL(glTextureSubImage3D(texture_array, 0, 0, 0, i, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, image))
            
            stbi_image_free(image);
        }
        CALLGL(glTextureParameteri(texture_array, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE))
        CALLGL(glTextureParameteri(texture_array, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE))
        CALLGL(glTextureParameteri(texture_array, GL_TEXTURE_MIN_FILTER, GL_LINEAR))
        CALLGL(glTextureParameteri(texture_array, GL_TEXTURE_MAG_FILTER, GL_LINEAR))
       
        CALLGL(glUseProgram(PR))
        GLint location = glGetUniformLocation(PR, "figures");
        if (location==-1) {
            LOGE("Could not locate uniform figures")
        }
        CALLGL(glUniform1i(location, 0))

        position_location = glGetUniformLocation(PR, "position");
        if (position_location==-1) {
            LOGE("Could not locate position")
        }
    }

    Board::Board(unsigned int board_size, dims view, unsigned int* position_data)
        : board_size_{board_size}, cells{board_size_*board_size_}, position{position_data} {
        
        init_buffers();
        init_gl_buffers(); 
        load_textures(view);     
    }

    void Board::apply () {
        
        CALLGL(glUseProgram(PR))
        CALLGL(glUniform1uiv(position_location, cells, position))

        CALLGL(glBindTextureUnit(0, texture_array))
        
        CALLGL(glBindVertexArray(VAO))
        CALLGL(glDrawElements(GL_TRIANGLES, i_size, GL_UNSIGNED_INT, 0))
        CALLGL(glBindVertexArray(0))

        CALLGL(glBindTextureUnit(0, 0))
    }

    Board::~Board () {

        delete[] vertices;
        delete[] indices;

        CALLGL(glDeleteShader(FS));
        CALLGL(glDeleteShader(VS));
        CALLGL(glDeleteProgram(PR));

        CALLGL(glDeleteTextures(1, &texture_array))
        CALLGL(glDeleteBuffers(1, &IBO))
        CALLGL(glDeleteBuffers(1, &VBO))
        CALLGL(glDeleteVertexArrays(1, &VAO))

        LOGD("Board destoyed")
    }

    const char* vert_src = R"(
        #version 460 core
        layout (location = 0) in vec3 aPos;
        void main() {
            gl_Position = vec4(aPos, 1.0);
        }
    )";

    const char* frag_src = R"(
        #version 460 core
        out vec4 FragColor;
        uniform vec3 color;
        void main() {
            FragColor = vec4(color, 1.0f);
        } 
    )";    

    Arrow::Arrow (float* buffer, size_t size)
        : buff_size{size}, arrow{buffer} {
        init_gl_buffers();
    }

    void Arrow::init_gl_buffers() {
        
        VS = gl::create_shader (vert_src, GL_VERTEX_SHADER);
        FS = gl::create_shader (frag_src, GL_FRAGMENT_SHADER);

        PR = gl::create_programm(VS, FS);

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);

        glLineWidth(5.0f);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, buff_size, nullptr, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
        glEnableVertexAttribArray(0);
        
        glUseProgram(PR);
        glUniform3f(glGetUniformLocation(PR, "color"), 0.0f, 0.0f, 1.0f);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void Arrow::apply() {

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, buff_size, arrow);

        glUseProgram(PR);
        glBindVertexArray(VAO);
        glDrawArrays(GL_LINES, 0, 2);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    Arrow::~Arrow() {

        glDeleteShader(FS);
        glDeleteShader(VS);
        glDeleteProgram(PR);

        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);
    }

} 
