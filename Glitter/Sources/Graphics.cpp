#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <iostream>
#include <stdexcept>

#include "Drawable.hpp"
#include "Graphics.hpp"
#include "FileMesh.hpp"
#include "PlanePrimitiveMesh.hpp"
#include "Shader.hpp"
#include "Timer.hpp"

using namespace glm;

Graphics::Graphics() {
    timer = std::make_shared<Timer>();
}

void Graphics::Init(ivec2 windowSize) {
    try {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        PlanePrimitiveMesh floorMesh(6.f);

        FileMesh monkeyMesh("suzanne.obj");

        monkeyShader = std::make_shared<Shader>();
        monkeyShader->AttachShader("drawing.vert");
        monkeyShader->AttachShader("drawing.frag");
        monkeyShader->Link();
        monkeyShader->ConnectUniforms(
            { "color", "shininess", "diffuseMix", "specularMix" }
        );

        monkey = std::make_unique<Drawable>(monkeyMesh, monkeyShader);

        auto floorShader = std::make_shared<Shader>();
        floorShader->AttachShader("drawing.vert");
        floorShader->AttachShader("drawing.frag");
        floorShader->Link();

        floor = std::make_unique<Drawable>(floorMesh, floorShader);

        auto cameraPos = vec3(2.5f, 2.5f, 2.5f);
        view = lookAt(
            cameraPos,
            vec3(0.f, 0.6f, 0.f),
            vec3(0.f, 1.f, 0.f)
        );
        UpdateAspect(windowSize);

        timer->Start();
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
}

void Graphics::Draw() {
    timer->Update();
    auto deltaTime = timer->GetDelta();
    auto time = timer->GetTime();

    // Background Fill Color
    glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!error.empty()) {
        ImGui::Begin("Error");
        ImGui::Text("%s", error.c_str());
        ImGui::End();
        return;
    }

    auto monkeyPos = vec3(0, 0.9f, 0);
    auto monkeyModelMat = translate(rotate(
        mat4(1.f), 
        time * radians(45.f), 
        vec3(0.f, 1.f, 0.f)
    ), monkeyPos);

    monkey->Draw(monkeyModelMat, view, proj);
    floor->Draw(mat4(1), view, proj);

    ImGui::Begin("Shader");
    float tempColor[3] = { color.r, color.g, color.b };
    ImGui::ColorPicker3("Colour", (float*) & tempColor, 0);
    color.x = tempColor[0];
    color.y = tempColor[1];
    color.z = tempColor[2];
    monkeyShader->SetUniform("color", color);

    ImGui::DragFloat("Shininess", &shininess, 0.1f, 0.f);
    monkeyShader->SetUniform("shininess", shininess);

    ImGui::SliderFloat("Diffuse", &diffuseMix, 0.f, 1.f);
    monkeyShader->SetUniform("diffuseMix", diffuseMix);

    ImGui::SliderFloat("Specular", &specularMix, 0.f, 1.f);
    monkeyShader->SetUniform("specularMix", specularMix);
    ImGui::End();

    ImGui::Begin("FPS");
    ImGui::Text("%.2f ms\n%.2f FPS", deltaTime * 1000.0f, 1.0f / deltaTime);
    ImGui::End();
}

Graphics::~Graphics() {
}
