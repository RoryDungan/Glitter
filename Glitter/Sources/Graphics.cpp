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
#include "Texture2D.hpp"
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

static void SetLight(Shader& shader, const Light& light, const mat4& lightSpaceMatrix, float penumbraSize) {
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
    shader.SetUniform("lightSpaceMatrix", lightSpaceMatrix);
    shader.SetUniform("penumbraSize", penumbraSize);
}

static const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;

static const float kCameraDistance = 5.f;

std::vector<Material> monkeyMats = {
    {   // Emerald
        vec3(0.0215f, 0.1745f, 0.0215f),
        vec3(0.07568f, 0.61424f, 0.07568f),
        vec3(0.633f, 0.727811f, 0.633f),
        0.6f
    },
    {   // Jade
        vec3(0.135f, 0.2225f, 0.0215f),
        vec3(0.54f, 0.89f, 0.63f),
        vec3(0.316228f),
        0.1f
    },
    {   // Obsidian
        vec3(0.05375f, 0.05f, 0.06625f),
        vec3(0.18275f, 0.17f, 0.22525f),
        vec3(0.332741f, 0.328634f, 0.346435f),
        0.3f
    },
    {   // Pearl
        vec3(0.25f, 0.20725f, 0.20725f),
        vec3(1.f, 0.829f, 0.829f),
        vec3(0.296648f),
        0.088f
    },
    {   // Ruby
        vec3(0.1745f, 0.01175f, 0.01175f),
        vec3(0.61424f, 0.04136f, 0.04136f),
        vec3(0.727811f, 0.626959f, 0.626959f),
        0.06f
    },
    {   // Green plastic
        vec3(0.f),
        vec3(0.1f, 0.35f, 0.1f),
        vec3(0.45f, 0.55f, 0.45f),
        0.25f
    },
    {   // White plastic
        vec3(0.f),
        vec3(0.55f),
        vec3(0.70f),
        0.25f
    },
    {   // Cyan rubber
        vec3(0.f, 0.05f, 0.05),
        vec3(0.4f, 0.5f, 0.5),
        vec3(0.04f, 0.7f, 0.7f),
        0.078125f
    },
    {   // Yellow rubber
        vec3(0.05f, 0.05f, 0.00f),
        vec3(0.5f, 0.5f, 0.4f),
        vec3(0.7f, 0.7f, 0.04f),
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

    mat4 cameraView = mat4(), cameraProj = mat4();

    ivec2 framebufferSize;

    vec3 lightStartPos = vec3(2.2f, 4.f, 2.f);
    float lightInnerCutoffDegrees = 35.f;
    float lightEdgeRadiusDegrees = 12.f;
    Light light = {
        lightStartPos,
        normalize(vec3(0.f) - lightStartPos), // look towards the center of the scene
        cos(radians(lightInnerCutoffDegrees)),
        cos(radians(lightInnerCutoffDegrees + lightEdgeRadiusDegrees)),

        vec3(0.2f),
        vec3(1.f),
        vec3(1.f),

        //1, 0.09, 0.032 // distance 50
        1.f, 0.027f, 0.0028f // distance 160
    };
    mat4 lightMat = mat4(1), lightSpaceMatrix = mat4(1);

    GLuint depthMapFBO = 0;
    std::unique_ptr<Texture2D> depthMap;
    std::shared_ptr<Shader> depthShader;
    float penumbraSize = 500.f;

    GLuint fbo = 0, rbo = 0, quadVAO = 0, quadVBO = 0;
    GLuint renderTexture = 0;

    std::unique_ptr<Shader> screenShader;

    std::string error;


    void InitDepthBuffer() {
        // Setup depth map
        glGenFramebuffers(1, &depthMapFBO);

        depthMap = std::make_unique<Texture2D>(
            uvec2(SHADOW_WIDTH, SHADOW_HEIGHT),
            Texture2D::DepthComponent,
            Texture2D::Float);
        depthMap->SetFiltering(Texture2D::Nearest);
        depthMap->SetWrapMode(Texture2D::ClampToBorder);
        depthMap->SetBorder(vec4(1.f, 1.f, 1.f, 1.f));

        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_DEPTH_ATTACHMENT,
            GL_TEXTURE_2D,
            depthMap->Get(),
            0
        );
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        depthShader = std::make_shared<Shader>();
        depthShader->AttachShader("depth.vert");
        depthShader->AttachShader("depth.frag");
        depthShader->Link();
    }

    void InitScene() {
        CubePrimitiveMesh lightMesh(.1f);
        pointLightShader = std::make_shared<Shader>();
        pointLightShader->AttachShader("drawing.vert");
        pointLightShader->AttachShader("light.frag");
        pointLightShader->Link();
        pointLightShader->ConnectUniforms({ "lightColor" });

        pointLightDrawable = std::make_unique<Drawable>(lightMesh, pointLightShader);

        //FileMesh monkeyMesh("suzanne.obj");
        //for (const Material& mat : monkeyMats) {
        //    auto shader = std::make_shared<Shader>();
        //    shader->AttachShader("drawing.vert");
        //    shader->AttachShader("solid-colour.frag");
        //    shader->Link();
        //    shader->ConnectUniforms({ 
        //        "material.ambient", 
        //        "material.diffuse", 
        //        "material.specular", 
        //        "material.shininess",
        //        "light.position", 
        //        "light.direction", 
        //        "light.cutOff", 
        //        "light.outerCutOff", 
        //        "light.ambient", 
        //        "light.diffuse", 
        //        "light.specular", 
        //        "light.constant", 
        //        "light.linear", 
        //        "light.quadratic", 
        //        "lightSpaceMatrix",
        //    });
        //    SetMat(*shader, mat);

        //    auto drawable = std::make_unique<Drawable>(monkeyMesh, shader);

        //    monkies.push_back(std::move(drawable));
        //    monkeyShaders.push_back(shader);
        //}
        auto shader = std::make_shared<Shader>();
        shader->AttachShader("drawing.vert");
        shader->AttachShader("textured.frag");
        shader->Link();
        shader->ConnectUniforms({ 
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
            "lightSpaceMatrix",
            "penumbraSize",
        });
        shader->SetUniform("material.shininess", 32.f);
        shader->InitTextures({
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
        });
        shader->ConnectDepthTex(depthMap->Get());

        CubePrimitiveMesh mesh(2.f);
        auto drawable = std::make_unique<Drawable>(mesh, shader);
        monkies.push_back(std::move(drawable));
        monkeyShaders.push_back(shader);


        PlanePrimitiveMesh floorMesh(12.f);
        floorShader = std::make_shared<Shader>();
        floorShader->AttachShader("drawing.vert");
        floorShader->AttachShader("textured.frag");
        floorShader->Link();
        floorShader->ConnectUniforms({ 
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
            "lightSpaceMatrix",
            "penumbraSize",
        });
        floorShader->SetUniform("material.shininess", 32.f);
        floorShader->InitTextures({
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
        });
        floorShader->ConnectDepthTex(depthMap->Get());

        floor = std::make_unique<Drawable>(floorMesh, floorShader);
    }

    void InitFramebuffer() {
        // Setup framebuffer
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        // create color attachment texture
        glGenTextures(1, &renderTexture);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, renderTexture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, framebufferSize.x, framebufferSize.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // bind texture to framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTexture, 0);

        // Create a renderbuffer objct for depth and stencil attachment
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, framebufferSize.x, framebufferSize.y);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
        // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            throw std::runtime_error("Framebuffer is not ready!");
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        screenShader = std::make_unique<Shader>();
        screenShader->AttachShader("framebuffer-display.vert");
        screenShader->AttachShader("framebuffer-display.frag");
        screenShader->Link();
        screenShader->ConnectUniforms({ 
            "screenTexture", 
            "time", 
            "clipPos"
        });
        screenShader->SetUniform("screenTexture", 0);

        float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
            // positions   // texCoords
            -1.0f,  1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
             1.0f, -1.0f,  1.0f, 0.0f,

            -1.0f,  1.0f,  0.0f, 1.0f,
             1.0f, -1.0f,  1.0f, 0.0f,
             1.0f,  1.0f,  1.0f, 1.0f
        };
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    }

    void InitView() {
        auto cameraPos = vec3(kCameraDistance);
        cameraView = lookAt(
            cameraPos,
            vec3(0.f, 0.6f, 0.f),
            vec3(0.f, 1.f, 0.f)
        );
        cameraProj = perspective(
            radians(45.f), 
            (float)framebufferSize.x / (float)framebufferSize.y, 
            1.f, 
            100.f
        );
    }

    void UpdateScene(float time) {
        // Move light
        lightMat = translate(
            rotate(
                mat4(1), 
                time * radians(-20.f), vec3(0, 1, 0)), 
            lightStartPos
        );
        pointLightShader->SetUniform("lightColor", light.specular);
        light.position = vec3(lightMat[3]);
        light.direction = normalize(vec3(0) - light.position);
        light.cutOff = cos(radians(lightInnerCutoffDegrees));
        light.outerCutOff = cos(radians(lightInnerCutoffDegrees + lightEdgeRadiusDegrees));

        for (auto shader : monkeyShaders) {
            SetLight(*shader, light, lightSpaceMatrix, penumbraSize);
        }

        SetLight(*floorShader, light, lightSpaceMatrix, penumbraSize);
    }

    void DrawFirstPass(mat4 view, mat4 proj, float time, std::shared_ptr<Shader> overrideShader = nullptr) {
        pointLightDrawable->Draw(lightMat, view, proj, overrideShader);
        // Draw monkeys
        for (size_t i = 0; i < monkies.size(); ++i) {
            const int rowSize = 3;
            const float spacing = 2.5f;
            auto monkeyPos = vec3(i / rowSize * spacing, 0.9f, i % rowSize * spacing) - vec3(2.5, 0, 2.5);

            //auto monkeyModelMat = rotate(
            //    translate(mat4(1.f), monkeyPos),
            //    time * radians(10.f),
            //    vec3(0.f, 1.f, 0.f)
            //);
            auto monkeyModelMat = translate(mat4(1.f), vec3(0.f, 1.0f, 0.f));

            monkies[i]->Draw(monkeyModelMat, view, proj, overrideShader);
        }
        
        // Draw floor
        floor->Draw(mat4(1), view, proj, overrideShader);
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

        if (ImGui::TreeNode("Shadow")) {
            ImGui::SliderFloat("Penumbra size", &penumbraSize, 1.f, 1000.f, "%.2f");
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

void Graphics::Init(ivec2 framebufferSize) {
    try {
        cc->framebufferSize = framebufferSize;

        glEnable(GL_CULL_FACE);

        cc->InitDepthBuffer();

        cc->InitScene();

        cc->InitFramebuffer();

        cc->InitView();

        // Done!
        cc->timer->Start();
    }
    catch (std::runtime_error& ex) {
        cc->error = ex.what();
        std::cerr << ex.what() << std::endl;
    }
}

void Graphics::OnResize(ivec2 framebufferSize) {
    cc->framebufferSize = framebufferSize;
    cc->cameraProj = perspective(
        radians(45.f), 
        (float)framebufferSize.x / (float)framebufferSize.y, 
        1.f, 
        100.f
    );

    // resize renderbuffer color attachment
    glBindTexture(GL_TEXTURE_2D, cc->renderTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, framebufferSize.x, framebufferSize.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);

    // resize depth attachment
    glBindRenderbuffer(GL_RENDERBUFFER, cc->rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, framebufferSize.x, framebufferSize.y);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void Graphics::Draw() {
    if (!cc->error.empty()) {
        ImGui::Begin("Error");
        ImGui::Text("%s", cc->error.c_str());
        ImGui::End();
    }

    cc->timer->Update();
    auto deltaTime = cc->timer->GetDelta();
    auto time = cc->timer->GetTime();

    auto lightView = lookAt(cc->light.position, cc->light.direction, vec3(0.f, 1.f, 0.f));
    auto lightProjection = perspective(
        radians(cc->lightInnerCutoffDegrees + cc->lightEdgeRadiusDegrees) * 2.f,
        (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT,
        1.f,
        10.f
    );
    //auto lightProjection = ortho(-8.f, 8.f, -8.f, 8.f, 0.5f, 10.f);
    cc->lightSpaceMatrix = lightProjection * lightView;

    cc->UpdateScene(time);

    glEnable(GL_DEPTH_TEST);
    // depth pass
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, cc->depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    cc->DrawFirstPass(lightView, lightProjection, time, cc->depthShader);
    // ConfigureShaderAndMatrices


    // first pass
    glViewport(0, 0, cc->framebufferSize.x, cc->framebufferSize.y);
    glBindFramebuffer(GL_FRAMEBUFFER, cc->fbo);
    glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    cc->DrawFirstPass(cc->cameraView, cc->cameraProj, time);

    // second pass
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    glClearColor(1.f, 1.f, 1.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    cc->screenShader->Activate();
    cc->screenShader->SetUniform("time", time);
    cc->screenShader->SetUniform("clipPos", vec4(0.f, 0.f, 1.f, 1.f));
    glBindVertexArray(cc->quadVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, cc->renderTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // GUI
    cc->DrawGUI(deltaTime);
}
