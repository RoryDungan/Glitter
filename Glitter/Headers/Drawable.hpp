#pragma once
#include <glm/glm.hpp>
#include "imgui.h"

#include "Mesh.hpp"
#include "Timer.hpp"

class Shader;

class Drawable {
public:
    Drawable(const Mesh& mesh, const std::shared_ptr<Shader> shader, const std::shared_ptr<Timer> timer) 
        : shaderProgram(shader), timer(timer) {
        glBindFragDataLocation(shaderProgram->Get(), 0, "outColor");

        numElements = mesh.GetNumElements();

        shaderProgram->Link();
        shaderProgram->Activate();

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vbo); // Generate 1 buffer
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, mesh.GetVertexDataSize(), mesh.GetVertexData(), GL_STATIC_DRAW);

        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.GetIndiciesSize(), mesh.GetIndices(), GL_STATIC_DRAW);

        shaderProgram->SetupVertexAttribs(mesh.VertexAttribs);

        shaderProgram->BindTextures({
            {
                "metal.jpg",
                "tex",
                {
                    {GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE},
                    {GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE},
                    {GL_TEXTURE_MIN_FILTER, GL_LINEAR},
                    {GL_TEXTURE_MAG_FILTER, GL_LINEAR}
                }
            },
            {
                "metal_norm.jpg",
                "normalMap",
                {
                    {GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE},
                    {GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE},
                    {GL_TEXTURE_MIN_FILTER, GL_LINEAR},
                    {GL_TEXTURE_MAG_FILTER, GL_LINEAR}
                }
            },
        });

        modelViewProjectionLocation =
            glGetUniformLocation(shaderProgram->Get(), "modelViewProjection");
        modelInverseTransposeLocation = 
            glGetUniformLocation(shaderProgram->Get(), "modelInverseTranspose");
        worldSpaceCameraPosLocation =
            glGetUniformLocation(shaderProgram->Get(), "worldSpaceCameraPos");
        reverseLightDirectionLocation = 
            glGetUniformLocation(shaderProgram->Get(), "reverseLightDirection");

        colorLocation = glGetUniformLocation(shaderProgram->Get(), "color");
        shininessLocation = glGetUniformLocation(shaderProgram->Get(), "shininess");
        diffuseMixLocation = glGetUniformLocation(shaderProgram->Get(), "diffuseMix");
        specularMixLocation = glGetUniformLocation(shaderProgram->Get(), "specularMix");

        model = glm::mat4(1.f);

        auto lightDir = normalize(glm::vec3(0.5, 0.7, 1));
        glUniform3fv(reverseLightDirectionLocation, 1, value_ptr(lightDir));
    }

    ~Drawable() {
        if (vbo != 0) {
            glDeleteBuffers(1, &vbo);
            vbo = 0;
        }
        if (vao != 0) {
            glDeleteVertexArrays(1, &vao);
            vao = 0;
        }
    }

    void Draw(glm::mat4 view, glm::mat4 projection) {
        auto time = timer->GetTime();

        shaderProgram->Activate();
        glBindVertexArray(vao);

        model = glm::rotate(
            glm::mat4(1.f), 
            time * glm::radians(45.f), 
            glm::vec3(0.f, 1.f, 0.f)
        );
        auto mvp = projection * view * model;
        glUniformMatrix4fv(modelViewProjectionLocation, 1, GL_FALSE, value_ptr(mvp));

        auto modelInverseTranspose = glm::mat3(transpose(inverse(model)));
        glUniformMatrix3fv(modelInverseTransposeLocation, 1, GL_FALSE, value_ptr(modelInverseTranspose));

        glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_INT, 0);


    ImGui::Begin("Shader");
    float tempColor[3] = { color.r, color.g, color.b };
    ImGui::ColorPicker3("Colour", (float*) & tempColor, 0);
    color.x = tempColor[0];
    color.y = tempColor[1];
    color.z = tempColor[2];
    glUniform3fv(colorLocation, 1, glm::value_ptr(color));

    ImGui::DragFloat("Shininess", &shininess, 0.1f, 0.f);
    glUniform1fv(shininessLocation, 1, &shininess);

    ImGui::SliderFloat("Diffuse", &diffuseMix, 0.f, 1.f);
    glUniform1fv(diffuseMixLocation, 1, &diffuseMix);

    ImGui::SliderFloat("Specular", &specularMix, 0.f, 1.f);
    glUniform1fv(specularMixLocation, 1, &specularMix);
    ImGui::End();
    }

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