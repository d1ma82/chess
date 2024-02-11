#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "window.h"
#include "gl_render.h"
#include "types.h"

namespace window {

    class GLFW final: public Window, private render::OpenGL {
    private:
        GLFWwindow* window {nullptr};
    public:
        GLFW (const GLFW& other) = delete;
        GLFW (GLFW&& other) = delete;
        GLFW& operator = (const GLFW& other) = delete;
        GLFW& operator = (GLFW&& other) = delete;

        GLFW(dims size, const char* title);

        bool should_close() { return glfwWindowShouldClose(window); }
        void close() { glfwSetWindowShouldClose(window, 1); }
        void draw();
        void set_key_event_listener(key_callback listener);
        void set_mouse_key_listener(mouse_click_callback listener);
        render::OpenGL* operator -> () { return static_cast<render::OpenGL*>(this); }
        ~GLFW();
    };
}