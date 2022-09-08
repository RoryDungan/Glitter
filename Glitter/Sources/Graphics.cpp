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

struct Material {
    vec3 ambient, diffuse, specular;
    float shininess;
};

static void SetMat(Shader& shader, const Material& mat) {
    shader.SetUniform("material.ambient", mat.ambient);
    shader.SetUniform("material.diffuse", mat.diffuse);
    shader.SetUniform("material.specular", mat.specular);
    shader.SetUniform("material.shininess", mat.shininess);
}

std::vector<Material> monkeyMats = {
    {   // Emerald
        vec3(0.0215, 0.1745, 0.0215),
        vec3(0.07568, 0.61424, 0.07568),
        vec3(0.633, 0.727811, 0.633),
        0.6f
    },
    {   // Jade
        vec3(0.135, 0.2225, 0.0215),
        vec3(0.54, 0.89, 0.63),
        vec3(0.316228),
        0.1f
    },
    {   // Obsidian
        vec3(0.05375, 0.05, 0.06625),
        vec3(0.18275, 0.17, 0.22525),
        vec3(0.332741, 0.328634, 0.346435),
        0.3f
    },
    {   // Pearl
        vec3(0.25, 0.20725, 0.20725),
        vec3(1, 0.829, 0.829),
        vec3(0.296648),
        0.088f
    },
    {   // Ruby
        vec3(0.1745, 0.01175, 0.01175),
        vec3(0.61424, 0.04136, 0.04136),
        vec3(0.727811, 0.626959, 0.626959),
        0.06f
    },
    {   // Green plastic
        vec3(0),
        vec3(0.1, 0.35, 0.1),
        vec3(0.45, 0.55, 0.45),
        0.25f
    },
    {   // White plastic
        vec3(0),
        vec3(0.55),
        vec3(0.70),
        0.25f
    },
    {   // Cyan rubber
        vec3(0, 0.05, 0.05),
        vec3(0.4, 0.5, 0.5),
        vec3(0.04, 0.7, 0.7),
        0.078125f
    },
    {   // Yellow rubber
        vec3(0.05, 0.05, 0.00),
        vec3(0.5, 0.5, 0.4),
        vec3(0.7, 0.7, 0.04),
        0.078125f
    },
};

struct Graphics::CheshireCat {
    std::unique_ptr<Timer> timer;
    std::vector<std::unique_ptr<Drawable>> monkies;
    std::unique_ptr<Drawable> floor;
    std::unique_ptr<Drawable> pointLightDrawable;

    std::shared_ptr<Shader> pointLightShader;
    std::vector<std::shared_ptr<Shader>> monkeyShaders;
    std::shared_ptr<Shader> floorShader;

    glm::mat4 view, proj;


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
        for (const Material& mat : monkeyMats) {
            auto shader = std::make_shared<Shader>();
            shader->AttachShader("drawing.vert");
            shader->AttachShader("solid-colour.frag");
            shader->Link();
            shader->ConnectUniforms({ 
                "material.ambient", 
                "material.diffuse", 
                "material.specular", 
                "material.shininess",
                "light.position", 
                "light.ambient", 
                "light.diffuse", 
                "light.specular", 
            });
            SetMat(*shader, mat);

            auto drawable = std::make_unique<Drawable>(monkeyMesh, shader);

            cc->monkies.push_back(std::move(drawable));
            cc->monkeyShaders.push_back(shader);
        }


        PlanePrimitiveMesh floorMesh(10.f);
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
            //{
            //    "matrix.jpg",
            //    "material.emission",
            //    {
            //        {GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE},
            //        {GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE},
            //        {GL_TEXTURE_MIN_FILTER, GL_LINEAR},
            //        {GL_TEXTURE_MAG_FILTER, GL_LINEAR}
            //    }
            //}
        });

        cc->floor = std::make_unique<Drawable>(floorMesh, cc->floorShader);

        auto cameraPos = vec3(10.f, 10.f, 10.f);
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
        100.f
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

    for (auto i = 0; i < cc->monkies.size(); ++i) {
        const int rowSize = 3;
        const float spacing = 2.5f;
        auto monkeyPos = vec3(i / rowSize * spacing, 0.9f, i % rowSize * spacing) - vec3(2.5, 0, 2.5);

        auto monkeyModelMat = rotate(
            translate(mat4(1.f), monkeyPos),
            time * radians(10.f),
            vec3(0.f, 1.f, 0.f)
        );

        cc->monkies[i]->Draw(monkeyModelMat, cc->view, cc->proj);
    }
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
    for (auto shader : cc->monkeyShaders) {
        shader->SetUniform("light.ambient", cc->lightColor * 0.2f);
        shader->SetUniform("light.diffuse", cc->lightColor * 0.5f);
        shader->SetUniform("light.specular", cc->lightColor);
        shader->SetUniform("light.position", cc->lightPos);
    }
    cc->floorShader->SetUniform("light.ambient", cc->lightColor * 0.2f);
    cc->floorShader->SetUniform("light.diffuse", cc->lightColor);
    cc->floorShader->SetUniform("light.specular", cc->lightColor);


    ImGui::End();

    ImGui::Begin("FPS");
    ImGui::Text("%.2f ms\n%.2f FPS", deltaTime * 1000.0f, 1.0f / deltaTime);
    ImGui::End();
}
