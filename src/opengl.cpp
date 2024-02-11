#include <vector>
#include <fstream>
#include <string>
#include <sstream>

#include "opengl.h"
#include "log.h"

namespace gl {
    
    GLenum err;

    void GLError(
        GLenum err, 
        int line,
        const char* file,
        const char* proc
    ) {
        const char* log;
        char buf[20];

        switch(err) {

            case GL_INVALID_ENUM: log ="GLenum argument out of range"; break;
            case GL_INVALID_VALUE: log = "numeric argument out of range"; break;
            case GL_INVALID_OPERATION: log="operation illegal in current state"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: log ="framebuffer object is not complete"; break;
            case GL_OUT_OF_MEMORY: log = "not enough memory left to execute command"; break;
            default: { snprintf(buf, sizeof(buf), "unlisted error %d", err); log=buf; }
        }
        LOGE("[FAIL GL] %s:%d, %s %s", file, line, proc, log)
    }

    GLuint create_shader(const char* src, GLenum type) {

        GLuint shader  = glCreateShader(type);
        
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);

        GLint is_compiled=0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &is_compiled);

        if (is_compiled==GL_FALSE) {

            GLint length=0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
            std::vector<GLchar> log(length);
            glGetShaderInfoLog(shader, length, &length, log.data());
            glDeleteShader(shader);
            std::ostringstream str;
            str << "Could not compile shader " << src << " - " << log.data();
            throw std::runtime_error(str.str().c_str());
        }
        return shader;
    }

    GLuint create_shader_file(const char* file, GLenum type) {

        std::ifstream  src(file);
        std::string    content;

        if (!src.is_open()) {
            
            std::ostringstream str;
            str << "Could not open file " << file;
            throw std::runtime_error(str.str().c_str());
        }
        src.seekg(0, std::ios::end);
        content.reserve(src.tellg());
        src.seekg(0, std::ios::beg);

        content.assign(std::istreambuf_iterator<char>(src), std::istreambuf_iterator<char>());
        src.close();

        return create_shader(content.c_str(), type);
    }

    GLuint create_programm(
        GLuint vertex_shader, 
        GLuint fragment_shader
    ) {
        GLuint prog = glCreateProgram();
        glAttachShader(prog, vertex_shader);
        glAttachShader(prog, fragment_shader);
        glLinkProgram(prog);
        GLint is_linked = 0;
        glGetProgramiv(prog, GL_LINK_STATUS, &is_linked);

        if (is_linked == GL_FALSE) {

            glDeleteProgram(prog);
            throw std::runtime_error("Could not link program");
        }
        return prog;
    }


    GLuint ext_texture() {

        GLuint result=0;
        #ifdef ANDROID
        glGenTextures(1, &result);
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, result);
        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        #endif
        return result;
    }


    GLuint d2_texture() {

        GLuint result=0;
        glGenTextures(1, &result);
        glBindTexture(GL_TEXTURE_2D, result);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
        return result;
    }

    GLuint d2_texture(GLsizei width, GLsizei height, GLint internal,  GLenum format, GLubyte* data) {
        GLuint result=0;
        glGenTextures(1, &result);
        glBindTexture(GL_TEXTURE_2D, result);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, internal, width, height, 0, format, GL_UNSIGNED_BYTE, (GLvoid*)data);
        glBindTexture(GL_TEXTURE_2D, 0);
        return result;    
    }
}