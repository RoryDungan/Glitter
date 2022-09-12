#include <glad/glad.h>
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

static const ImGuiColorEditFlags ColorEditFlags = ImGuiColorEditFlags_PickerHueWheel;

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
    float lightInnerCutoffDegrees = 50.f;
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

    GLuint fbo = 0, rbo = 0, quadVAO = 0, quadVBO = 0;
    GLuint renderTexture = 0;

    std::unique_ptr<Shader> screenShader;

    std::string error;


    void DrawFirstPass(float deltaTime, float time) {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glEnable(GL_DEPTH_TEST);

        // Background Fill Color
        glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (!error.empty()) {
            ImGui::Begin("Error");
            ImGui::Text("%s", error.c_str());
            ImGui::End();
            return;
        }

        // Move light
        auto lightMat = translate(
            rotate(
                mat4(1), 
                time * radians(-20.f), vec3(0, 1, 0)), 
            lightStartPos
        );
        pointLightShader->SetUniform("lightColor", light.specular);
        pointLightDrawable->Draw(lightMat, view, proj);
        light.position = vec3(lightMat[3]);
        light.direction = normalize(vec3(0) - light.position);
        light.cutOff = cos(radians(lightInnerCutoffDegrees));
        light.outerCutOff = cos(radians(lightInnerCutoffDegrees + lightEdgeRadiusDegrees));

        // Draw monkeys
        for (auto shader : monkeyShaders) {
            SetLight(*shader, light);
        }
        for (size_t i = 0; i < monkies.size(); ++i) {
            const int rowSize = 3;
            const float spacing = 2.5f;
            auto monkeyPos = vec3(i / rowSize * spacing, 0.9f, i % rowSize * spacing) - vec3(2.5, 0, 2.5);

            auto monkeyModelMat = rotate(
                translate(mat4(1.f), monkeyPos),
                time * radians(10.f),
                vec3(0.f, 1.f, 0.f)
            );

            monkies[i]->Draw(monkeyModelMat, view, proj);
        }
        
        // Draw floor
        SetLight(*floorShader, light);
        floor->Draw(mat4(1), view, proj);
    }

    void DrawGUI(float deltaTime) {
        ImGui::Begin("Shader");
        float tempColor[3];

        tempColor[0] = light.specular.r;
        tempColor[1] = light.specular.g;
        tempColor[2] = light.specular.b;
        ImGui::ColorEdit3("Light colour", (float*) &tempColor, ColorEditFlags);
        light.specular.r = tempColor[0];
        light.specular.g = tempColor[1];
        light.specular.b = tempColor[2];
        light.ambient = light.specular * 0.2f;
        light.diffuse = light.specular;

        if (ImGui::TreeNode("Attenuation")) {
            ImGui::DragFloat("Constant", &light.constant, 0.01f, 0.f, 100.f, "%.5f");
            ImGui::DragFloat("Linear", &light.linear, 0.01f, 0.f, 1.f, "%.5f");
            ImGui::DragFloat("Quadratic", &light.quadratic, 0.01f, 0.f, 1.f, "%.5f");

            const int numSamples = 100;

            static float maxDistance = 50.f;

            float graph[numSamples];
            for (int i = 0; i < numSamples; ++i) {
                const float dist = (float)i / (float)numSamples * maxDistance;
                graph[i] = 1.f / (light.constant + light.linear * dist + light.quadratic * dist * dist);
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
            ImGui::SliderFloat("Radius", &lightInnerCutoffDegrees, 0.f, 180.f, "%.2f°");
            ImGui::SliderFloat("Edge size", &lightEdgeRadiusDegrees, 0.f, 180.f, "%.2f°");
            ImGui::TreePop();
        }

        ImGui::End();

        ImGui::Begin("FPS");
        ImGui::Text("%.2f ms\n%.2f FPS", deltaTime * 1000.0f, 1.0f / deltaTime);
        ImGui::End();
    }
};

Graphics::Graphics() : cc(std::make_unique<CheshireCat>()) {
    cc->timer = std::make_unique<Timer>();
}

Graphics::~Graphics() {
    if (cc->fbo != 0) {
        glDeleteFramebuffers(1, &cc->fbo);
        cc->fbo = 0;
    }
}

void Graphics::Init(ivec2 windowSize) {
    try {
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



        // Setup framebuffer
        glGenFramebuffers(1, &cc->fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, cc->fbo);

        // create color attachment texture
        glGenTextures(1, &cc->renderTexture);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, cc->renderTexture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowSize.x, windowSize.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // bind texture to framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, cc->renderTexture, 0);

        // Create a renderbuffer objct for depth and stencil attachment
        glGenRenderbuffers(1, &cc->rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, cc->rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowSize.x, windowSize.y);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, cc->rbo);
        // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            throw std::runtime_error("Framebuffer is not ready!");
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        cc->screenShader = std::make_unique<Shader>();
        cc->screenShader->AttachShader("framebuffer-display.vert");
        cc->screenShader->AttachShader("framebuffer-display.frag");
        cc->screenShader->Link();
        cc->screenShader->ConnectUniforms({ 
            "screenTexture", 
            "time", 
            "clipPos"
        });
        cc->screenShader->SetUniform("screenTexture", 0);

        float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
            // positions   // texCoords
            -1.0f,  1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
             1.0f, -1.0f,  1.0f, 0.0f,

            -1.0f,  1.0f,  0.0f, 1.0f,
             1.0f, -1.0f,  1.0f, 0.0f,
             1.0f,  1.0f,  1.0f, 1.0f
        };
        glGenVertexArrays(1, &cc->quadVAO);
        glGenBuffers(1, &cc->quadVBO);
        glBindVertexArray(cc->quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, cc->quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

        cc->proj = perspective(
            radians(45.f), 
            (float)windowSize.x / (float)windowSize.y, 
            1.f, 
            100.f
        );

        // Done!
        cc->timer->Start();
    }
    catch (std::runtime_error& ex) {
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

    // resize renderbuffer color attachment
    glBindTexture(GL_TEXTURE_2D, cc->renderTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowSize.x, windowSize.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);

    // resize depth attachment
    glBindRenderbuffer(GL_RENDERBUFFER, cc->rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowSize.x, windowSize.y);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void Graphics::Draw() {
    cc->timer->Update();
    auto deltaTime = cc->timer->GetDelta();
    auto time = cc->timer->GetTime();

    cc->DrawFirstPass(deltaTime, time);

    // second pass
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    glClearColor(1.f, 1.f, 1.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    cc->screenShader->Activate();
    cc->screenShader->SetUniform("time", time);
    glBindVertexArray(cc->quadVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, cc->renderTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);


    // GUI
    cc->DrawGUI(deltaTime);
}
