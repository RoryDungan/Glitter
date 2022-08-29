#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <algorithm>
#include <iostream>
#include <glm/glm.hpp>
#include <sstream>
#include <stdexcept>
#include "Graphics.hpp"
#include "stb_image.h"
#include "imgui.h"

using namespace glm;

void Graphics::Init(ivec2 windowSize) {
    try {
        Assimp::Importer importer;

        const auto* filename = "suzanne.obj";
        const auto* scene = importer.ReadFile(filename,
            aiProcess_CalcTangentSpace |
            aiProcess_Triangulate |
            aiProcess_JoinIdenticalVertices |
            aiProcess_SortByPType);

        if (scene == nullptr) {
            std::ostringstream s;
            s << "Could not load model file \"" << filename << '"';
            throw std::runtime_error(s.str());
        }
        if (scene->mNumMeshes <= 0) {
            std::ostringstream s;
            s << "Could not fine any meshes in model file \"" << filename << '"';
            throw std::runtime_error(s.str());
        }

        auto* mesh = scene->mMeshes[0];
        const auto componentsPerVertex = 14;
        std::vector<float> vertices(mesh->mNumVertices * componentsPerVertex);
        for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
            vertices[i * componentsPerVertex] = mesh->mVertices[i].x;
            vertices[i * componentsPerVertex + 1] = mesh->mVertices[i].y;
            vertices[i * componentsPerVertex + 2] = mesh->mVertices[i].z;
            vertices[i * componentsPerVertex + 3] = mesh->mNormals[i].x;
            vertices[i * componentsPerVertex + 4] = mesh->mNormals[i].y;
            vertices[i * componentsPerVertex + 5] = mesh->mNormals[i].z;
            vertices[i * componentsPerVertex + 6] = mesh->mTangents[i].x;
            vertices[i * componentsPerVertex + 7] = mesh->mTangents[i].y;
            vertices[i * componentsPerVertex + 8] = mesh->mTangents[i].z;
            vertices[i * componentsPerVertex + 9] = mesh->mBitangents[i].x;
            vertices[i * componentsPerVertex + 10] = mesh->mBitangents[i].y;
            vertices[i * componentsPerVertex + 11] = mesh->mBitangents[i].z;
            vertices[i * componentsPerVertex + 12] = mesh->mTextureCoords[0][i].x;
            vertices[i * componentsPerVertex + 13] = mesh->mTextureCoords[0][i].y;
        }

        std::vector<unsigned int> indices(mesh->mNumFaces * 3);
        for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
            indices[i * 3] = mesh->mFaces[i].mIndices[0];
            indices[i * 3 + 1] = mesh->mFaces[i].mIndices[1];
            indices[i * 3 + 2] = mesh->mFaces[i].mIndices[2];
        }
        numElements = mesh->mNumFaces * 3;

        glEnable(GL_DEPTH_TEST);

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vbo); // Generate 1 buffer
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(float) * indices.size(), indices.data(), GL_STATIC_DRAW);

        shaderProgram = std::make_unique<Shader>();
        shaderProgram->AttachShader("drawing.vert");
        shaderProgram->AttachShader("drawing.frag");

        glBindFragDataLocation(shaderProgram->Get(), 0, "outColor");

        shaderProgram->Link();
        shaderProgram->Activate();

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
        view = lookAt(
            vec3(0.f, 1.5f, 1.5f),
            vec3(0.f, 0.f, 0.f),
            vec3(0.f, 0.f, 1.f)
        );
        UpdateAspect(windowSize);

        auto modelInverseTranspose = mat3(transpose(inverse(model)));
        glUniformMatrix3fv(modelInverseTransposeLocation, 1, GL_FALSE, value_ptr(modelInverseTranspose));

        // set the light direction.
        auto uniReverseLightDirection = glGetUniformLocation(shaderProgram->Get(), "reverseLightDirection");
        auto lightDir = normalize(vec3(0.5, 0.7, 1));
        glUniform3fv(uniReverseLightDirection, 1, value_ptr(lightDir));

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
    model = rotate(model, time * radians(45.f), vec3(0.f, 0.f, 1.f));
    auto mvp = proj * view * model;
    glUniformMatrix4fv(modelViewProjectionLocation, 1, GL_FALSE, value_ptr(mvp));
    auto modelInverseTranspose = mat3(transpose(inverse(model)));
    glUniformMatrix3fv(modelInverseTransposeLocation, 1, GL_FALSE, value_ptr(modelInverseTranspose));
    
    glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    ImGui::Begin("FPS");
    ImGui::Text("%.2f ms\n%.2f FPS", deltaTime * 1000.0f, 1.0f / deltaTime);
    ImGui::End();
}

Graphics::~Graphics() {
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}
