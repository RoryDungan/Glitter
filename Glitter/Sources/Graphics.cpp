#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <iostream>
#include <stdexcept>

#include "CubePrimitiveMesh.hpp"
#include "Drawable.hpp"
#include "Graphics.hpp"
#include "FileMesh.hpp"
#include "PlanePrimitiveMesh.hpp"
#include "Shader.hpp"
#include "Timer.hpp"

using namespace glm;

struct Graphics::CheshireCat {
    std::unique_ptr<Timer> timer;
    std::unique_ptr<Drawable> monkey;
    std::unique_ptr<Drawable> floor;
    std::unique_ptr<Drawable> pointLightDrawable;

    std::shared_ptr<Shader> pointLightShader;
    std::shared_ptr<Shader> monkeyShader;
    std::shared_ptr<Shader> floorShader;

    glm::mat4 view, proj;

    glm::vec3 color = glm::vec3(0.5f, 1.f, 0.5f);
    glm::vec3 lightColor = glm::vec3(1, 1, 1);
    glm::vec3 lightStartPos = vec3(1.2f, 2.f, 1.f);
    glm::vec3 lightPos = lightStartPos;

    float shininess = 10.f, diffuseMix = 1.f, specularMix = 1.f, normalsMix = 1.f;

    std::string error;
};

Graphics::Graphics() : cc(std::make_unique<CheshireCat>()) {
    cc->timer = std::make_unique<Timer>();
}

Graphics::~Graphics() = default;

void Graphics::Init(ivec2 windowSize) {
    try {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        CubePrimitiveMesh lightMesh(.1f);
        cc->pointLightShader = std::make_shared<Shader>();
        cc->pointLightShader->AttachShader("drawing.vert");
        cc->pointLightShader->AttachShader("light.frag");
        cc->pointLightShader->Link();
        cc->pointLightShader->ConnectUniforms({ "lightColor" });

        cc->pointLightDrawable = std::make_unique<Drawable>(lightMesh, cc->pointLightShader);

        FileMesh monkeyMesh("suzanne.obj");
        cc->monkeyShader = std::make_shared<Shader>();
        cc->monkeyShader->AttachShader("drawing.vert");
        cc->monkeyShader->AttachShader("solid-colour.frag");
        cc->monkeyShader->Link();
        cc->monkeyShader->ConnectUniforms(
            { "color", "shininess", "diffuseMix", "specularMix", "normalMapMix", "lightColor", "lightPos"}
        );

        cc->monkey = std::make_unique<Drawable>(monkeyMesh, cc->monkeyShader);

        PlanePrimitiveMesh floorMesh(3.f);
        cc->floorShader = std::make_shared<Shader>();
        cc->floorShader->AttachShader("drawing.vert");
        cc->floorShader->AttachShader("textured.frag");
        cc->floorShader->Link();
        cc->floorShader->ConnectUniforms(
            { "color", "shininess", "diffuseMix", "specularMix", "normalMapMix", "lightColor", "lightPos"}
        );
        cc->floorShader->InitTextures({
            {
                "brickwall.jpg",
                "tex",
                {
                    {GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE},
                    {GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE},
                    {GL_TEXTURE_MIN_FILTER, GL_LINEAR},
                    {GL_TEXTURE_MAG_FILTER, GL_LINEAR}
                }
            },
            {
                "brickwall_normal.jpg",
                "normalMap",
                {
                    {GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE},
                    {GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE},
                    {GL_TEXTURE_MIN_FILTER, GL_LINEAR},
                    {GL_TEXTURE_MAG_FILTER, GL_LINEAR}
                }
            },
        });

        cc->floor = std::make_unique<Drawable>(floorMesh, cc->floorShader);

        auto cameraPos = vec3(2.5f, 2.5f, 2.5f);
        cc->view = lookAt(
            cameraPos,
            vec3(0.f, 0.6f, 0.f),
            vec3(0.f, 1.f, 0.f)
        );
        UpdateAspect(windowSize);

        cc->timer->Start();
    }
    catch (std::runtime_error ex) {
        cc->error = ex.what();
        std::cerr << ex.what() << std::endl;
    }
}

void Graphics::UpdateAspect(ivec2 windowSize) {
    cc->proj = perspective(
        radians(45.f), 
        (float)windowSize.x / (float)windowSize.y, 
        1.f, 
        10.f
    );
}

void Graphics::Draw() {
    cc->timer->Update();
    auto deltaTime = cc->timer->GetDelta();
    auto time = cc->timer->GetTime();

    // Background Fill Color
    glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!cc->error.empty()) {
        ImGui::Begin("Error");
        ImGui::Text("%s", cc->error.c_str());
        ImGui::End();
        return;
    }


    auto lightMat = translate(rotate(mat4(1), time * radians(-20.f), vec3(0, 1, 0)), cc->lightStartPos);
    cc->pointLightDrawable->Draw(lightMat, cc->view, cc->proj);
    cc->lightPos = vec3(lightMat[3]);

    auto monkeyPos = vec3(0, 0.9f, 0);
    auto monkeyModelMat = translate(rotate(
        mat4(1.f), 
        time * radians(10.f), 
        vec3(0.f, 1.f, 0.f)
    ), monkeyPos);

    cc->monkeyShader->SetUniform("lightPos", cc->lightPos);
    cc->monkey->Draw(monkeyModelMat, cc->view, cc->proj);
    cc->floorShader->SetUniform("lightPos", cc->lightPos);
    cc->floor->Draw(mat4(1), cc->view, cc->proj);

    ImGui::Begin("Shader");
    float tempColor[3] = { cc->color.r, cc->color.g, cc->color.b };
    ImGui::ColorPicker3("Colour", (float*) & tempColor, 0);
    cc->color.x = tempColor[0];
    cc->color.y = tempColor[1];
    cc->color.z = tempColor[2];
    cc->monkeyShader->SetUniform("color", cc->color);

    tempColor[0] = cc->lightColor.r;
    tempColor[1] = cc->lightColor.g;
    tempColor[2] = cc->lightColor.b;
    ImGui::ColorPicker3("Light colour", (float*) & tempColor, 0);
    cc->lightColor.x = tempColor[0];
    cc->lightColor.y = tempColor[1];
    cc->lightColor.z = tempColor[2];
    cc->monkeyShader->SetUniform("lightColor", cc->lightColor);
    cc->pointLightShader->SetUniform("lightColor", cc->lightColor);

    ImGui::DragFloat("Shininess", &cc->shininess, 0.1f, 0.f);
    cc->monkeyShader->SetUniform("shininess", cc->shininess);

    ImGui::SliderFloat("Diffuse", &cc->diffuseMix, 0.f, 1.f);
    cc->monkeyShader->SetUniform("diffuseMix", cc->diffuseMix);

    ImGui::SliderFloat("Specular", &cc->specularMix, 0.f, 1.f);
    cc->monkeyShader->SetUniform("specularMix", cc->specularMix);
    //ImGui::SliderFloat("Normal map", &normalsMix, 0.f, 1.f);
    //monkeyShader->SetUniform("normalMapMix", normalsMix);

    ImGui::End();

    ImGui::Begin("FPS");
    ImGui::Text("%.2f ms\n%.2f FPS", deltaTime * 1000.0f, 1.0f / deltaTime);
    ImGui::End();
}
