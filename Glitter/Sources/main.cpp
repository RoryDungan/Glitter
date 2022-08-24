// Local Headers
#include "glitter.hpp"

// System Headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <memory>

#include "Graphics.hpp"
#include "Window.hpp"

int main(int argc, char * argv[]) {
    auto graphics = std::make_shared<Graphics>();

    Window window(graphics, mWidth, mHeight, "OpenGL");
    graphics->Init(glm::ivec2(mWidth, mHeight));
    window.LoopUntilDone();

    return EXIT_SUCCESS;
}
