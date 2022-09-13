// System Headers
#include <glm/glm.hpp>
#include <memory>

#include "Graphics.hpp"
#include "Window.hpp"

int main(int argc, char * argv[]) {
    auto graphics = std::make_shared<Graphics>();

    const auto width = 1280;
    const auto height = 800;

    Window window(graphics, width, height, "OpenGL");
    graphics->Init(window.GetFramebufferSize());
    window.LoopUntilDone();

    return EXIT_SUCCESS;
}
