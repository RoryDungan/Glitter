#pragma once

#include <memory>
#include <glm/glm.hpp>
#include <string>

#include "glitter.hpp"
#include "Drawable.hpp"
#include "Shader.hpp"
#include "Timer.hpp"

class Graphics {
public:
    Graphics() { 
        timer = std::make_shared<Timer>();
    }

    void Init(glm::ivec2 windowSize);
    void Draw();

    void UpdateAspect(glm::ivec2 windowSize);

    ~Graphics();
private:
    std::shared_ptr<Timer> timer;
    std::unique_ptr<Drawable> monkey;

    glm::mat4 view, proj;

    glm::vec3 color = glm::vec3(0.5f, 1.f, 0.5f);
    float shininess = 10.f, diffuseMix = 1.f, specularMix = 1.f;

    std::string error;
};
