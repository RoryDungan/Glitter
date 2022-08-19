#pragma once
#include "glitter.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <memory>

#include "Graphics.hpp"

class Window
{
public:
    Window(std::shared_ptr<Graphics> graphics, int width, int height, const char* title) {
        Window::graphics = graphics;

        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
        window = glfwCreateWindow(width, height, "OpenGL", nullptr, nullptr);
        glfwSetFramebufferSizeCallback(window, Window::OnResize);

        // TODO
        // Check for Valid Context
        //if (window == nullptr) {
        //    fprintf(stderr, "Failed to Create OpenGL Context");
        //    return EXIT_FAILURE;
        //}

        glfwMakeContextCurrent(window);
        gladLoadGL();
        fprintf(stderr, "OpenGL %s\n", glGetString(GL_VERSION));
    }

    void LoopUntilDone() {
        while (!glfwWindowShouldClose(window)) {
            Draw();
        }

        glfwTerminate();
    }

private:
    static void OnResize(GLFWwindow* window, int width, int height)
    {
        glViewport(0, 0, width, height);
        Draw();
    }

    static void Draw() {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        graphics->Draw();

        // Flip Buffers and Draw
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    static GLFWwindow* window;
    static std::shared_ptr<Graphics> graphics;
};
