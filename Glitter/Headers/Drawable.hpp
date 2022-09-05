#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>

#include "Mesh.hpp"
#include "Timer.hpp"

class Shader;

class Drawable {
public:
    Drawable(const Mesh& mesh, const std::shared_ptr<Shader> shader, const std::shared_ptr<Timer> timer);

    ~Drawable();

    void Draw(glm::mat4 view, glm::mat4 projection);

private:
    std::shared_ptr<Shader> shaderProgram;
    std::shared_ptr<Timer> timer;

    // uniform locations
    GLint modelViewProjectionLocation = 0;
    GLint modelInverseTransposeLocation = 0;
    GLint worldSpaceCameraPosLocation = 0; 
    GLint reverseLightDirectionLocation = 0;
    GLint colorLocation = 0;
    GLint shininessLocation = 0;
    GLint diffuseMixLocation = 0; 
    GLint specularMixLocation = 0;

    GLuint vbo = 0, vao = 0, ebo = 0;

    glm::mat4 model;

    glm::vec3 color = glm::vec3(0.5f, 1.f, 0.5f);
    float shininess = 10.f, diffuseMix = 1.f, specularMix = 1.f;

    unsigned int numElements;
};