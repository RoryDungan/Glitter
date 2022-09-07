#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

#include "Drawable.hpp"
#include "Mesh.hpp"
#include "Shader.hpp"
#include "Timer.hpp"


using namespace glm;

Drawable::Drawable(const Mesh& mesh, const std::shared_ptr<Shader> shader) 
    : shaderProgram(shader) {
    glBindFragDataLocation(shaderProgram->Get(), 0, "outColor");

    numElements = mesh.GetNumElements();

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo); // Generate 1 buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh.GetVertexDataSize(), mesh.GetVertexData(), GL_STATIC_DRAW);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.GetIndiciesSize(), mesh.GetIndices(), GL_STATIC_DRAW);

    shaderProgram->Activate();

    shaderProgram->SetupVertexAttribs(mesh.GetVertexAttribs());

    modelViewProjectionLocation =
        glGetUniformLocation(shaderProgram->Get(), "modelViewProjection");
    modelInverseTransposeLocation = 
        glGetUniformLocation(shaderProgram->Get(), "modelInverseTranspose");
    worldSpaceCameraPosLocation =
        glGetUniformLocation(shaderProgram->Get(), "worldSpaceCameraPos");
    reverseLightDirectionLocation = 
        glGetUniformLocation(shaderProgram->Get(), "reverseLightDirection");

    auto lightDir = normalize(vec3(0.5, 0.7, 1));
    glUniform3fv(reverseLightDirectionLocation, 1, value_ptr(lightDir));
}

Drawable::~Drawable() {
    if (vbo != 0) {
        glDeleteBuffers(1, &vbo);
        vbo = 0;
    }
    if (vao != 0) {
        glDeleteVertexArrays(1, &vao);
        vao = 0;
    }
    if (ebo != 0) {
        glDeleteBuffers(1, &ebo);
        ebo = 0;
    }
}

void Drawable::Draw(mat4 model, mat4 view, mat4 projection) {

    shaderProgram->Activate();
    shaderProgram->BindTextures();
    glBindVertexArray(vao);

    auto mvp = projection * view * model;
    glUniformMatrix4fv(modelViewProjectionLocation, 1, GL_FALSE, value_ptr(mvp));

    auto modelInverseTranspose = mat3(transpose(inverse(model)));
    glUniformMatrix3fv(modelInverseTransposeLocation, 1, GL_FALSE, value_ptr(modelInverseTranspose));
    //glUniformMatrix3fv(modelInverseTransposeLocation, 1, GL_FALSE, value_ptr(model));

    glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_INT, 0);
}