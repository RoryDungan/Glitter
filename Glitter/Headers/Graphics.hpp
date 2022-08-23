#pragma once

#include <chrono>
#include <memory>

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

    GLuint vbo = 0, vao = 0;
};