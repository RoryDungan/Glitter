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

    glm::vec3 ambient = glm::vec3(0.0215, 0.1745, 0.0215);
    glm::vec3 diffuse = glm::vec3(0.07568, 0.61424, 0.07568);
    glm::vec3 specular = glm::vec3(0.633, 0.727811, 0.633);
    float shininess = 0.6f;

    glm::vec3 lightColor = glm::vec3(1, 1, 1);
    glm::vec3 lightStartPos = vec3(1.2f, 2.f, 1.f);
    glm::vec3 lightPos = lightStartPos;

    std::string error;
};

static const ImGuiColorEditFlags ColorEditFlags = ImGuiColorEditFlags_PickerHueWheel;

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
        cc->monkeyShader->ConnectUniforms({ 
            "material.ambient", 
            "material.diffuse", 
            "material.specular", 
            "material.shininess",
            "light.position", 
            "light.ambient", 
            "light.diffuse", 
            "light.specular", 
        });

        cc->monkey = std::make_unique<Drawable>(monkeyMesh, cc->monkeyShader);

        PlanePrimitiveMesh floorMesh(3.f);
        cc->floorShader = std::make_shared<Shader>();
        cc->floorShader->AttachShader("drawing.vert");
        cc->floorShader->AttachShader("textured.frag");
        cc->floorShader->Link();
        cc->floorShader->ConnectUniforms({ 
            "material.specular", 
            "material.shininess", 
            "light.position", 
            "light.ambient", 
            "light.diffuse", 
            "light.specular", 
        });
        cc->floorShader->SetUniform("material.shininess", 32.f);
        cc->floorShader->InitTextures({
            {
                "container2.png",
                "material.diffuse",
                {
                    {GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE},
                    {GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE},
                    {GL_TEXTURE_MIN_FILTER, GL_LINEAR},
                    {GL_TEXTURE_MAG_FILTER, GL_LINEAR}
                }
            },
            {
                "container2_specular.png",
                "material.specular",
                {
                    {GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE},
                    {GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE},
                    {GL_TEXTURE_MIN_FILTER, GL_LINEAR},
                    {GL_TEXTURE_MAG_FILTER, GL_LINEAR}
                }
            },
            {
                "blue.png",
                "material.normal",
                {
                    {GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE},
                    {GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE},
                    {GL_TEXTURE_MIN_FILTER, GL_LINEAR},
                    {GL_TEXTURE_MAG_FILTER, GL_LINEAR}
                }
            },
            {
                "matrix.jpg",
                "material.emission",
                {
                    {GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE},
                    {GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE},
                    {GL_TEXTURE_MIN_FILTER, GL_LINEAR},
                    {GL_TEXTURE_MAG_FILTER, GL_LINEAR}
                }
            }
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

    cc->monkeyShader->SetUniform("light.position", cc->lightPos);
    cc->monkey->Draw(monkeyModelMat, cc->view, cc->proj);
    cc->floorShader->SetUniform("light.position", cc->lightPos);
    cc->floor->Draw(mat4(1), cc->view, cc->proj);

    ImGui::Begin("Shader");
    float tempColor[3];

    tempColor[0] = cc->lightColor.r;
    tempColor[1] = cc->lightColor.g;
    tempColor[2] = cc->lightColor.b;
    ImGui::ColorEdit3("Light colour", (float*) & tempColor, ColorEditFlags);
    cc->lightColor.r = tempColor[0];
    cc->lightColor.g = tempColor[1];
    cc->lightColor.b = tempColor[2];
    cc->pointLightShader->SetUniform("lightColor", cc->lightColor);
    cc->monkeyShader->SetUniform("light.ambient", cc->lightColor * 0.2f);
    cc->monkeyShader->SetUniform("light.diffuse", cc->lightColor * 0.5f);
    cc->monkeyShader->SetUniform("light.specular", cc->lightColor);
    cc->floorShader->SetUniform("light.ambient", cc->lightColor * 0.2f);
    cc->floorShader->SetUniform("light.diffuse", cc->lightColor);
    cc->floorShader->SetUniform("light.specular", cc->lightColor);


    // material props
    tempColor[0] = cc->ambient.r;
    tempColor[1] = cc->ambient.g;
    tempColor[2] = cc->ambient.b;
    ImGui::ColorEdit3("ambient", (float*) & tempColor, ColorEditFlags);
    cc->ambient.r = tempColor[0];
    cc->ambient.g = tempColor[1];
    cc->ambient.b = tempColor[2];
    cc->monkeyShader->SetUniform("material.ambient", cc->ambient);

    tempColor[0] = cc->diffuse.r;
    tempColor[1] = cc->diffuse.g;
    tempColor[2] = cc->diffuse.b;
    ImGui::ColorEdit3("diffuse", (float*) & tempColor, ColorEditFlags);
    cc->diffuse.r = tempColor[0];
    cc->diffuse.g = tempColor[1];
    cc->diffuse.b = tempColor[2];
    cc->monkeyShader->SetUniform("material.diffuse", cc->diffuse);

    tempColor[0] = cc->specular.r;
    tempColor[1] = cc->specular.g;
    tempColor[2] = cc->specular.b;
    ImGui::ColorEdit3("specular", (float*) & tempColor, ColorEditFlags);
    cc->specular.r = tempColor[0];
    cc->specular.g = tempColor[1];
    cc->specular.b = tempColor[2];
    cc->monkeyShader->SetUniform("material.specular", cc->specular);

    ImGui::DragFloat("Shininess", &cc->shininess, 0.1f, 0.f);
    cc->monkeyShader->SetUniform("material.shininess", cc->shininess);

    ImGui::End();

    ImGui::Begin("FPS");
    ImGui::Text("%.2f ms\n%.2f FPS", deltaTime * 1000.0f, 1.0f / deltaTime);
    ImGui::End();
}
