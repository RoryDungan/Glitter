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

    void Init();
    void Draw();

    ~Graphics();
private:
    std::chrono::steady_clock::time_point startTime;
    std::chrono::steady_clock::time_point lastFrameTime;
    std::unique_ptr<Shader> shaderProgram;
    
    GLint uniModel;

    GLuint vbo = 0, vao = 0;

    std::string error;
};
