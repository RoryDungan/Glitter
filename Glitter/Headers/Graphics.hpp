#pragma once

#include <glm/glm.hpp>
#include <memory>

class Graphics {
public:
    Graphics(); 
    ~Graphics(); 
    void Init(glm::uvec2 framebufferSize);
    void Draw();

    void OnResize(glm::uvec2 framebufferSize);
    void OnCursorMoved(glm::dvec2 cursorPos);
    void OnMouseButton(int button, int action);

private:
    struct CheshireCat;
    const std::unique_ptr<CheshireCat> cc;
};
