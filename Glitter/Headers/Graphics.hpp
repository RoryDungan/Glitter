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
    
    glm::mat4 trans = glm::mat4(1.0f);
    GLint uniTrans;

    GLuint vbo = 0, vao = 0;

    std::string error;
};
