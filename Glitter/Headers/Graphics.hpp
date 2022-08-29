#pragma once

#include <memory>
#include <glm/glm.hpp>
#include <string>

#include "glitter.hpp"
#include "Shader.hpp"
#include "Timer.hpp"

class Graphics {
public:
    Graphics() { }

    void Init(glm::ivec2 windowSize);
    void Draw();

    void UpdateAspect(glm::ivec2 windowSize);

    ~Graphics();
private:
    Timer timer;
    std::unique_ptr<Shader> shaderProgram;
    
    // uniform locations
    GLint modelViewProjectionLocation, modelInverseTransposeLocation;

    GLuint vbo = 0, vao = 0, ebo = 0;
    
    unsigned int numElements;

    glm::mat4 model, view, proj;

    std::string error;
};
