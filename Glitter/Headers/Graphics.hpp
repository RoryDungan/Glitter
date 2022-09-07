#pragma once

#include <glm/glm.hpp>
#include <memory>

class Graphics {
public:
    Graphics(); 
    ~Graphics(); 
    void Init(glm::ivec2 windowSize);
    void Draw();

    void UpdateAspect(glm::ivec2 windowSize);

private:
    struct CheshireCat;
    std::unique_ptr<CheshireCat> cc;
};
