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

struct Light {
    vec3 position, direction;

    float cutOff, outerCutOff;

    vec3 ambient, diffuse, specular;

    float constant, linear, quadratic;
};

static void SetMat(Shader& shader, const Material& mat) {
    shader.SetUniform("material.ambient", mat.ambient);
    shader.SetUniform("material.diffuse", mat.diffuse);
    shader.SetUniform("material.specular", mat.specular);
    shader.SetUniform("material.shininess", mat.shininess);
}

static void SetLight(Shader& shader, const Light& light) {
    shader.SetUniform("light.position", light.position);
    shader.SetUniform("light.direction", light.direction);
    shader.SetUniform("light.cutOff", light.cutOff);
    shader.SetUniform("light.outerCutOff", light.outerCutOff);
    shader.SetUniform("light.ambient", light.ambient);
    shader.SetUniform("light.diffuse", light.diffuse);
    shader.SetUniform("light.specular", light.specular);
    shader.SetUniform("light.constant", light.constant);
    shader.SetUniform("light.linear", light.linear);
    shader.SetUniform("light.quadratic", light.quadratic);
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


    glm::vec3 lightStartPos = vec3(2.2f, 4.f, 2.f);
    float lightInnerCutoffDegrees = 25.f;
    float lightEdgeRadiusDegrees = 5.f;
    Light light = {
        lightStartPos,
        normalize(vec3(0) - lightStartPos), // look towards the center of the scene
        cos(radians(lightInnerCutoffDegrees)),
        cos(radians(lightInnerCutoffDegrees + lightEdgeRadiusDegrees)),

        vec3(0.2),
        vec3(1),
        vec3(1),

        //1, 0.09, 0.032 // distance 50
        1, 0.027, 0.0028 // distance 160
    };

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
                "light.direction", 
                "light.cutOff", 
                "light.outerCutOff", 
                "light.ambient", 
                "light.diffuse", 
                "light.specular", 
                "light.constant", 
                "light.linear", 
                "light.quadratic", 
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
            "light.direction", 
            "light.cutOff", 
            "light.outerCutOff", 
            "light.ambient", 
            "light.diffuse", 
            "light.specular", 
            "light.constant", 
            "light.linear", 
            "light.quadratic", 
        });
        cc->floorShader->SetUniform("material.shininess", 32.f);
        cc->floorShader->InitTextures({
            {
                "brickwall.jpg",
                "material.diffuse",
                {
                    {GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE},
                    {GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE},
                    {GL_TEXTURE_MIN_FILTER, GL_LINEAR},
                    {GL_TEXTURE_MAG_FILTER, GL_LINEAR}
                }
            },
            //{
            //    "container2_specular.png",
            //    "material.specular",
            //    {
            //        {GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE},
            //        {GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE},
            //        {GL_TEXTURE_MIN_FILTER, GL_LINEAR},
            //        {GL_TEXTURE_MAG_FILTER, GL_LINEAR}
            //    }
            //},
            {
                "brickwall_normal.jpg",
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

    // Move light
    auto lightMat = translate(
        rotate(
            mat4(1), 
            time * radians(-20.f), vec3(0, 1, 0)), 
        cc->lightStartPos
    );
    cc->pointLightShader->SetUniform("lightColor", cc->light.specular);
    cc->pointLightDrawable->Draw(lightMat, cc->view, cc->proj);
    cc->light.position = vec3(lightMat[3]);
    cc->light.direction = normalize(vec3(0) - cc->light.position);
    cc->light.cutOff = cos(radians(cc->lightInnerCutoffDegrees));
    cc->light.outerCutOff = cos(radians(cc->lightInnerCutoffDegrees + cc->lightEdgeRadiusDegrees));

    // Draw monkeys
    for (auto shader : cc->monkeyShaders) {
        SetLight(*shader, cc->light);
    }
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
    
    // Draw floor
    SetLight(*cc->floorShader, cc->light);
    cc->floor->Draw(mat4(1), cc->view, cc->proj);

    // GUI
    ImGui::Begin("Shader");
    float tempColor[3];

    tempColor[0] = cc->light.specular.r;
    tempColor[1] = cc->light.specular.g;
    tempColor[2] = cc->light.specular.b;
    ImGui::ColorEdit3("Light colour", (float*) & tempColor, ColorEditFlags);
    cc->light.specular.r = tempColor[0];
    cc->light.specular.g = tempColor[1];
    cc->light.specular.b = tempColor[2];
    cc->light.ambient = cc->light.specular * 0.2f;
    cc->light.diffuse = cc->light.specular;

    if (ImGui::TreeNode("Attenuation")) {
        ImGui::DragFloat("Constant", &cc->light.constant, 0.01f, 0.f, 100.f, "%.5f");
        ImGui::DragFloat("Linear", &cc->light.linear, 0.01f, 0.f, 1.f, "%.5f");
        ImGui::DragFloat("Quadratic", &cc->light.quadratic, 0.01f, 0.f, 1.f, "%.5f");

        const int numSamples = 100;

        static float maxDistance = 50.f;

        float graph[numSamples];
        for (int i = 0; i < numSamples; ++i) {
            const float dist = (float)i / (float)numSamples * maxDistance;
            graph[i] = 1.f / (cc->light.constant + cc->light.linear * dist + cc->light.quadratic * dist * dist);
        }
        ImGui::PlotLines(
            "Falloff",
            graph,
            numSamples,
            0,
            "",
            0.f,
            1.f,
            ImVec2(0.f, 80.f)
        );
        ImGui::SliderFloat("preview scale", &maxDistance, 1.f, 100.f);

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Cutoff")) {
        ImGui::SliderFloat("Radius", &cc->lightInnerCutoffDegrees, 0.f, 180.f, "%.2f°");
        ImGui::SliderFloat("Edge size", &cc->lightEdgeRadiusDegrees, 0.f, 180.f, "%.2f°");
        ImGui::TreePop();
    }

    ImGui::End();

    ImGui::Begin("FPS");
    ImGui::Text("%.2f ms\n%.2f FPS", deltaTime * 1000.0f, 1.0f / deltaTime);
    ImGui::End();
}
