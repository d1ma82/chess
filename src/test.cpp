#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include "log.h"

const char *vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec2 aTexCoord;
    layout (location = 2) in vec3 aColor;

    out vec2 TexCoord;
    out vec3 ourColor;

    void main()
    {
        gl_Position = vec4(aPos, 1.0);
        TexCoord = aTexCoord;
        ourColor = aColor;
    }
)";

const char *fragmentShaderSource = R"(
    #version 330 core
    in vec2 TexCoord;
    in vec3 ourColor;

    out vec4 FragColor;

    void main()
    {
        FragColor = vec4(ourColor, 1.0f);
    }
)";

int main() {
    // Initialize GLFW and create a window
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Example", nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
            LOGE("Failed to init GLAD")
            throw std::exception();
    }

    // Get the number of rectangles (NxN)
    int N;
    std::cout << "Enter the number of rectangles (N): ";
    std::cin >> N;

    // Validate input
    if (N <= 0) {
        std::cerr << "Invalid input. N must be a positive integer." << std::endl;
        return -1;
    }

    // Define vertices, texture coordinates, and colors
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    float rectSize = 2.0f / static_cast<float>(N);

    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            // Calculate position for each rectangle
            float x = -1.0f + j * rectSize;
            float y = 1.0f - i * rectSize;

            // Calculate color based on the position
            float colorValue = ((i + j) % 2 == 0) ? 1.0f : 0.0f;

            // Add vertices
            vertices.push_back(x); vertices.push_back(y); vertices.push_back(0.0f);        // Position
            vertices.push_back(0.0f); vertices.push_back(0.0f);                             // Texture Coordinates
            vertices.push_back(colorValue); vertices.push_back(colorValue); vertices.push_back(colorValue);  // Color
        }
    }

    // Generate indices
    for (int i = 0; i < N - 1; ++i) {
        for (int j = 0; j < N - 1; ++j) {
            unsigned int topLeft = i * N + j;
            unsigned int topRight = i * N + j + 1;
            unsigned int bottomLeft = (i + 1) * N + j;
            unsigned int bottomRight = (i + 1) * N + j + 1;

            indices.push_back(topLeft);
            indices.push_back(topRight);
            indices.push_back(bottomLeft);

            indices.push_back(topRight);
            indices.push_back(bottomRight);
            indices.push_back(bottomLeft);
        }
    }

    // Create Vertex Buffer Object (VBO)
    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // Create Index Buffer Object (IBO)
    GLuint IBO;
    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Create Vertex Array Object (VAO)
    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Color attribute
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Create vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    // Create fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    // Create shader program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &IBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}
