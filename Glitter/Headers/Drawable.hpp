#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>

class Shader;
class Mesh;
class Timer;

class Drawable {
public:
    Drawable(const Mesh& mesh, const std::shared_ptr<Shader> shader);

    virtual ~Drawable();

    void Draw(
        const glm::mat4& model, 
        const glm::mat4& view, 
        const glm::mat4& projection, 
        std::shared_ptr<Shader> overrideShader = nullptr
    ) const;

private:
    std::shared_ptr<Shader> shaderProgram;

    // uniform locations
    GLint modelLocation = 0;
    GLint modelViewProjectionLocation = 0;
    GLint modelInverseTransposeLocation = 0;
    GLint worldSpaceCameraPosLocation = 0; 
    GLint reverseLightDirectionLocation = 0;

    GLuint vbo = 0, vao = 0, ebo = 0;

    glm::vec3 color = glm::vec3(0.5f, 1.f, 0.5f);

    unsigned int numElements;
};
