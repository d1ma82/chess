#pragma once
#include "filter.h"
#include "opengl.h"
#include "log.h"
#include "types.h"
#include <vector>

namespace render {
    
    class OpenGL {
    private:
        std::vector<filter::Base*> filters;
        GLint err;
    public:
        OpenGL() {}

        void dump_version() {
            LOGI("GL_VERSION: %s, GL_SHADER_LANG_VERSION: %s", 
                    glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION))
        }

        void viewport (dims view) {
            
            glViewport(0, 0, view.first, view.second);
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        }

        void run() { 

            if (filters.empty()) return;
            CALLGL(glClear(GL_COLOR_BUFFER_BIT))
            for (auto f: filters) f->apply();
        }
        void attach_filter(filter::Base* filter) { filters.push_back(filter); }
    };
}