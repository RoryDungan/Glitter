#pragma once

#include <chrono>
#include <memory>
#include <glm/glm.hpp>
#include <string>

#include "glitter.hpp"
#include "Shader.hpp"

class Graphics {
public:
    Graphics() { }

    void Init(glm::ivec2 windowSize);
    void Draw();

    void UpdateAspect(glm::ivec2 windowSize);

    ~Graphics();
private:
    std::chrono::steady_clock::time_point startTime;
    std::chrono::steady_clock::time_point lastFrameTime;
    std::unique_ptr<Shader> shaderProgram;
    
    // uniform locations
    GLint modelLocation, modelViewProjectionLocation, modelInverseTransposeLocation;

    GLuint vbo = 0, vao = 0;

    glm::mat4 model, view, proj;

    std::string error;
};
