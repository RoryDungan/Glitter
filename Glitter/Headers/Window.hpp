#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <memory>

#include "Graphics.hpp"

class Window
{
public:
    Window(std::shared_ptr<Graphics> graphics, int width, int height, const char* title);
    
    void LoopUntilDone();

    glm::ivec2 GetFramebufferSize() const;
    
private:
    static void OnResize(GLFWwindow* window, int width, int height);

    static void Draw();

    static GLFWwindow* window;
    static std::shared_ptr<Graphics> graphics;
};
