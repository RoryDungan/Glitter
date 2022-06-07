// Local Headers
#include "glitter.hpp"

// System Headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Standard Headers
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>

#include <filesystem>
#include <iostream>

// wish I didn't have to do this lol
std::chrono::steady_clock::time_point t_start;
GLFWwindow* mWindow;

void draw() {
    if (glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(mWindow, true);

    // Background Fill Color
    glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    auto t_now = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();

    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Flip Buffers and Draw
    glfwSwapBuffers(mWindow);
    glfwPollEvents();
}

bool readShaderFile(GLuint shader, GLsizei count, const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file) {
        return false;
    }
    std::stringstream sstr;
    sstr << file.rdbuf();
    auto file_contents = sstr.str();
    const char* shaderSource = file_contents.c_str();

    glShaderSource(shader, count, &shaderSource, NULL);
    return true;
}

int main(int argc, char * argv[]) {
    auto exe_path = std::filesystem::path(argv[0]);
    std::cout << exe_path << std::endl;
    std::cout << std::filesystem::current_path() << std::endl;

    // Load GLFW and Create a Window
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    mWindow = glfwCreateWindow(mWidth, mHeight, "OpenGL", nullptr, nullptr);
    glfwSetFramebufferSizeCallback(mWindow, [](GLFWwindow* window, int width, int height) {
        glViewport(0, 0, width, height);
        draw();
    });

    // Check for Valid Context
    if (mWindow == nullptr) {
        fprintf(stderr, "Failed to Create OpenGL Context");
        return EXIT_FAILURE;
    }

    // Create Context and Load OpenGL Functions
    glfwMakeContextCurrent(mWindow);
    gladLoadGL();
    fprintf(stderr, "OpenGL %s\n", glGetString(GL_VERSION));


    float vertices[] = {
         0.0f,  0.5f, 1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
        -0.5f, -0.5f, 0.0f, 0.0f, 1.0f
    };

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    GLuint vbo;
    glGenBuffers(1, &vbo); // Generate 1 buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);


    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    exe_path.replace_filename("drawing.vert");
    if (!readShaderFile(vertexShader, 1, exe_path)) {
        return 1;
    }
    glCompileShader(vertexShader);
    GLint status;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        char buffer[512];
        glGetShaderInfoLog(vertexShader, 512, NULL, buffer);
        printf(buffer);

        return 1;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    exe_path.replace_filename("drawing.frag");
    if (!readShaderFile(fragmentShader, 1, exe_path)) {
        return 1;
    }
    glCompileShader(fragmentShader);

    //GLint status;
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        char buffer[512];
        glGetShaderInfoLog(fragmentShader, 512, NULL, buffer);
        printf(buffer);

        return 1;
    }

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glBindFragDataLocation(shaderProgram, 0, "outColor");

    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), 0);

    GLint colAttrib = glGetAttribLocation(shaderProgram, "color");
    glEnableVertexAttribArray(colAttrib);
    glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));

    t_start = std::chrono::high_resolution_clock::now();


    // Rendering Loop
    while (glfwWindowShouldClose(mWindow) == false) {
        draw();
    }   glfwTerminate();

    glDeleteProgram(shaderProgram);
    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);

    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);

    return EXIT_SUCCESS;
}
