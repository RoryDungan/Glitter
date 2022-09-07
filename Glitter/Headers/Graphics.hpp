#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <string>

class Drawable;
class Shader;
class Timer;

class Graphics {
public:
    Graphics(); 
    ~Graphics(); 
    void Init(glm::ivec2 windowSize);
    void Draw();

    void UpdateAspect(glm::ivec2 windowSize);

private:
    std::unique_ptr<Timer> timer;
    std::unique_ptr<Drawable> monkey;
    std::unique_ptr<Drawable> floor;
    std::unique_ptr<Drawable> pointLightDrawable;

    std::shared_ptr<Shader> pointLightShader;
    std::shared_ptr<Shader> monkeyShader;

    glm::mat4 view, proj;

    glm::vec3 color = glm::vec3(0.5f, 1.f, 0.5f);
    glm::vec3 lightColor = glm::vec3(1, 1, 1);
    float shininess = 10.f, diffuseMix = 1.f, specularMix = 1.f, normalsMix = 1.f;

    std::string error;
};
