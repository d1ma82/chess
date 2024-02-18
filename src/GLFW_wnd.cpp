#include "GLFW_wnd.h"
#include "log.h"
#include <stdexcept>

namespace window {

    GLFW::GLFW (dims size, const char* title) {
        
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        glfwWindowHint(GLFW_SAMPLES, 16);

        window = glfwCreateWindow(size.first, size.second, title, nullptr, nullptr);
        if (window==nullptr) throw std::runtime_error ("Failed to create GLFW window");
        
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);
        if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) throw std::runtime_error ("Failed to init GLAD");
        
       this->dump_version(); 
       this->viewport(size);     // Cannot use render::OpenGL construtor cause it Use GL functions before GL context
    }

    void GLFW::set_key_event_listener(key_callback listener) {

        static key_callback callback = listener;

        glfwSetKeyCallback(window, [] 
            (GLFWwindow*, int key, int, int action, int) { 
               callback(key, action); 
        });
    }

    void GLFW::set_mouse_key_listener(mouse_click_callback listener) {

        static mouse_click_callback callback = listener;

        glfwSetMouseButtonCallback(window, [](GLFWwindow* wnd, int button, int action, int) {

            if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
                
                double xpos, ypos;
                glfwGetCursorPos(wnd, &xpos, &ypos);
                callback(xpos, ypos);
            }
        });
    }

    void GLFW::draw() {

        this->run();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    GLFW::~GLFW() {

        glfwTerminate();
        LOGD("Destroy window")
    }
}
