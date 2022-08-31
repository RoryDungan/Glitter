#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <algorithm>
#include <array>
#include <iostream>
#include <glm/glm.hpp>
#include <sstream>
#include <stdexcept>
#include "Graphics.hpp"
#include "Mesh.hpp"
#include "stb_image.h"
#include "imgui.h"

using namespace glm;

template<std::size_t DESTOFFSET, std::size_t DESTSIZE>
void staticCopyToVector(const aiVector3t<float>& vert, std::array<float, DESTSIZE>& dstVector) {
    static_assert(DESTOFFSET + 3 <= DESTSIZE);
    dstVector[DESTOFFSET] = vert.x;
    dstVector[DESTOFFSET + 1] = vert.y;
    dstVector[DESTOFFSET + 2] = vert.z;
}

void Graphics::Init(ivec2 windowSize) {
    try {
        Assimp::Importer importer;

        const auto* filename = "suzanne.obj";

        Mesh mesh(filename);
        numElements = mesh.GetNumElements();

        glEnable(GL_DEPTH_TEST);

        shaderProgram = std::make_unique<Shader>();
        shaderProgram->AttachShader("drawing.vert");
        shaderProgram->AttachShader("drawing.frag");

        glBindFragDataLocation(shaderProgram->Get(), 0, "outColor");

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

        shaderProgram->SetupVertexAttribs({
            {"position", 3, GL_FLOAT},
            {"normal", 3, GL_FLOAT},
            {"tangent", 3, GL_FLOAT},
            {"bitangent", 3, GL_FLOAT},
            {"texcoord", 2, GL_FLOAT},
        });
        
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

        model = mat4(1.f);
        auto cameraPos = vec3(2.5f, 1.5f, 2.5f);
        view = lookAt(
            cameraPos,
            vec3(0.f, 0.f, 0.f),
            vec3(0.f, 1.f, 0.f)
        );
        UpdateAspect(windowSize);

        auto worldSpaceCameraPosLocation =
            glGetUniformLocation(shaderProgram->Get(), "worldSpaceCameraPos");
        glUniform3fv(worldSpaceCameraPosLocation, 1, value_ptr(cameraPos));

        auto modelInverseTranspose = mat3(transpose(inverse(model)));
        glUniformMatrix3fv(modelInverseTransposeLocation, 1, GL_FALSE, value_ptr(modelInverseTranspose));

        // set the light direction.
        auto uniReverseLightDirection = glGetUniformLocation(shaderProgram->Get(), "reverseLightDirection");
        auto lightDir = normalize(vec3(0.5, 0.7, 1));
        glUniform3fv(uniReverseLightDirection, 1, value_ptr(lightDir));


        colorLocation = glGetUniformLocation(shaderProgram->Get(), "color");
        shininessLocation = glGetUniformLocation(shaderProgram->Get(), "shininess");
        diffuseMixLocation = glGetUniformLocation(shaderProgram->Get(), "diffuseMix");
        specularMixLocation = glGetUniformLocation(shaderProgram->Get(), "specularMix");

        timer.Start();
    }
    catch (std::runtime_error ex) {
        error = ex.what();
        std::cerr << ex.what() << std::endl;
    }
}

void Graphics::UpdateAspect(ivec2 windowSize) {
    proj = perspective(
        radians(45.f), 
        (float)windowSize.x / (float)windowSize.y, 
        1.f, 
        10.f
    );
    auto mvp = proj * view * model;
    glUniformMatrix4fv(modelViewProjectionLocation, 1, GL_FALSE, value_ptr(mvp));
}

void Graphics::Draw() {
    timer.Update();
    auto time = timer.GetTime();
    auto deltaTime = timer.GetDelta();

    // Background Fill Color
    glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!error.empty()) {
        ImGui::Begin("Error");
        ImGui::Text("%s", error.c_str());
        ImGui::End();
        return;
    }


    shaderProgram->Activate();
    glBindVertexArray(vao);

    model = mat4(1.0f);
    model = rotate(model, time * radians(45.f), vec3(0.f, 1.f, 0.f));
    auto mvp = proj * view * model;
    glUniformMatrix4fv(modelViewProjectionLocation, 1, GL_FALSE, value_ptr(mvp));
    auto modelInverseTranspose = mat3(transpose(inverse(model)));
    glUniformMatrix3fv(modelInverseTransposeLocation, 1, GL_FALSE, value_ptr(modelInverseTranspose));
    
    glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    ImGui::Begin("FPS");
    ImGui::Text("%.2f ms\n%.2f FPS", deltaTime * 1000.0f, 1.0f / deltaTime);
    ImGui::End();

    ImGui::Begin("Shader");
    float tempColor[3] = { color.r, color.g, color.b };
    ImGui::ColorPicker3("Colour", (float*) & tempColor, 0);
    color.x = tempColor[0];
    color.y = tempColor[1];
    color.z = tempColor[2];
    glUniform3fv(colorLocation, 1, value_ptr(color));

    ImGui::DragFloat("Shininess", &shininess, 0.1f, 0.f);
    glUniform1fv(shininessLocation, 1, &shininess);

    ImGui::SliderFloat("Diffuse", &diffuseMix, 0.f, 1.f);
    glUniform1fv(diffuseMixLocation, 1, &diffuseMix);

    ImGui::SliderFloat("Specular", &specularMix, 0.f, 1.f);
    glUniform1fv(specularMixLocation, 1, &specularMix);
    ImGui::End();
}

Graphics::~Graphics() {
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}
