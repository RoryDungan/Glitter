#pragma once

#include <glm/glm.hpp>
#include <memory>

class Graphics {
public:
    Graphics(); 
    ~Graphics(); 
    void Init(glm::ivec2 framebufferSize);
    void Draw();

    void OnResize(glm::ivec2 framebufferSize);

private:
    struct CheshireCat;
    const std::unique_ptr<CheshireCat> cc;
};
