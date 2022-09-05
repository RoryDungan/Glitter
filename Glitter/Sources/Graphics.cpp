#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <iostream>
#include <stdexcept>

#include "Drawable.hpp"
#include "Graphics.hpp"
#include "FileMesh.hpp"
#include "Shader.hpp"
#include "Timer.hpp"

using namespace glm;

Graphics::Graphics() {
    timer = std::make_shared<Timer>();
}

void Graphics::Init(ivec2 windowSize) {
    try {
        glEnable(GL_DEPTH_TEST);

        FileMesh mesh("suzanne.obj");

        auto shaderProgram = std::make_shared<Shader>();
        shaderProgram->AttachShader("drawing.vert");
        shaderProgram->AttachShader("drawing.frag");

        monkey = std::make_unique<Drawable>(mesh, shaderProgram, timer);

        auto cameraPos = vec3(2.5f, 1.5f, 2.5f);
        view = lookAt(
            cameraPos,
            vec3(0.f, 0.f, 0.f),
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

    // Background Fill Color
    glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!error.empty()) {
        ImGui::Begin("Error");
        ImGui::Text("%s", error.c_str());
        ImGui::End();
        return;
    }

    monkey->Draw(view, proj);

    ImGui::Begin("FPS");
    ImGui::Text("%.2f ms\n%.2f FPS", deltaTime * 1000.0f, 1.0f / deltaTime);
    ImGui::End();
}

Graphics::~Graphics() {
}
