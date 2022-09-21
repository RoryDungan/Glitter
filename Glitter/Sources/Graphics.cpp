#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <iostream>
#include <stdexcept>
#include <stb_image.h>
#include <iostream>

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

static const float kCameraDistance = 2.f;

static const ImGuiColorEditFlags ColorEditFlags = ImGuiColorEditFlags_PickerHueWheel;

struct Graphics::CheshireCat {
    std::unique_ptr<Timer> timer;
    std::vector<std::shared_ptr<Drawable>> characterDrawables;
    std::unique_ptr<Drawable> floor;
    std::unique_ptr<Drawable> pointLightDrawable;

    std::shared_ptr<Shader> pointLightShader;
    std::vector<std::shared_ptr<Shader>> sceneShaders;
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
    std::shared_ptr<Texture2D> depthMap;
    std::shared_ptr<Shader> depthShader;
    float penumbraSize = 500.f;

    GLuint fbo = 0, rbo = 0, quadVAO = 0, quadVBO = 0;
    std::shared_ptr<Texture2D> renderTexture = 0;

    std::unique_ptr<Shader> screenShader;

    std::string error;


    void InitDepthBuffer() {
        // Setup depth map
        glGenFramebuffers(1, &depthMapFBO);

        depthMap = std::make_shared<Texture2D>(
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

        pointLightDrawable = std::make_unique<Drawable>(lightMesh, pointLightShader);

        auto shader = std::make_shared<Shader>();
        shader->AttachShader("drawing.vert");
        shader->AttachShader("textured.frag");
        shader->Link();
        shader->SetUniform("material.shininess", 32.f);

        auto characterDiffuse = std::make_shared<Texture2D>("TP_Guide_S0_DF.png");
        characterDiffuse->SetWrapMode(Texture2D::ClampToEdge);
        characterDiffuse->SetFiltering(Texture2D::Linear);
        shader->AddTexture("material.diffuse", characterDiffuse);

        auto characterNormalMap = std::make_shared<Texture2D>("TP_Guide_S0_NM.png");
        characterNormalMap->SetWrapMode(Texture2D::ClampToEdge);
        characterNormalMap->SetFiltering(Texture2D::Linear);
        shader->AddTexture("material.normal", characterNormalMap);

        auto characterSpecular = std::make_shared<Texture2D>("black.png");
        characterSpecular->SetWrapMode(Texture2D::ClampToEdge);
        characterSpecular->SetFiltering(Texture2D::Linear);
        shader->AddTexture("material.specular", characterSpecular);

        shader->AddTexture("shadowMap", depthMap);

        auto hairShader = std::make_shared<Shader>();
        hairShader->AttachShader("drawing.vert");
        hairShader->AttachShader("textured.frag");
        hairShader->Link();
        hairShader->SetUniform("material.shininess", 32.f);

        auto hairDiffuse = std::make_shared<Texture2D>("TP_Guide_S0_Hair_DF.png");
        hairDiffuse->SetWrapMode(Texture2D::ClampToEdge);
        hairDiffuse->SetFiltering(Texture2D::Linear);
        hairShader->AddTexture("material.diffuse", hairDiffuse);

        auto hairNormalMap = std::make_shared<Texture2D>("TP_Guide_S0_Hair_NM.png");
        hairNormalMap->SetWrapMode(Texture2D::ClampToEdge);
        hairNormalMap->SetFiltering(Texture2D::Linear);
        hairShader->AddTexture("material.normal", hairNormalMap);

        hairShader->AddTexture("material.specular", characterSpecular);
        hairShader->AddTexture("shadowMap", depthMap);

        characterDrawables = {
            std::make_shared<Drawable>(FileMesh("Skye.obj", 0), shader),
            std::make_shared<Drawable>(FileMesh("Skye.obj", 1), shader),
            std::make_shared<Drawable>(FileMesh("Skye.obj", 2), hairShader),
        };
        sceneShaders.push_back(shader);
        sceneShaders.push_back(hairShader);


        PlanePrimitiveMesh floorMesh(12.f);
        floorShader = std::make_shared<Shader>();
        floorShader->AttachShader("drawing.vert");
        floorShader->AttachShader("textured.frag");
        floorShader->Link();
        floorShader->SetUniform("material.shininess", 32.f);

        auto floorDiffuse = std::make_shared<Texture2D>("brickwall.jpg");
        floorDiffuse->SetWrapMode(Texture2D::ClampToEdge);
        floorDiffuse->SetFiltering(Texture2D::Linear);
        floorShader->AddTexture("material.diffuse", floorDiffuse);

        auto floorNormal = std::make_shared<Texture2D>("brickwall_normal.jpg");
        floorNormal->SetWrapMode(Texture2D::ClampToEdge);
        floorNormal->SetFiltering(Texture2D::Linear);
        floorShader->AddTexture("material.normal", floorNormal);

        floorShader->AddTexture("shadowMap", depthMap);

        floor = std::make_unique<Drawable>(floorMesh, floorShader);
    }

    void InitFramebuffer() {
        // Setup framebuffer
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        // create color attachment texture
        renderTexture = std::make_shared<Texture2D>(framebufferSize, Texture2D::RGB, Texture2D::UnsignedByte);
        renderTexture->SetFiltering(Texture2D::Linear);

        // bind texture to framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTexture->Get(), 0);

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
        auto cameraPos = vec3(kCameraDistance / 3.f, kCameraDistance, kCameraDistance);
        cameraView = lookAt(
            cameraPos,
            vec3(0.f, 1.3f, 0.f),
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

        for (auto shader : sceneShaders) {
            SetLight(*shader, light, lightSpaceMatrix, penumbraSize);
        }

        SetLight(*floorShader, light, lightSpaceMatrix, penumbraSize);
    }

    void DrawFirstPass(mat4 view, mat4 proj, float time, std::shared_ptr<Shader> overrideShader = nullptr) {
        pointLightDrawable->Draw(lightMat, view, proj, overrideShader);
        // Draw character
        for (auto& d : characterDrawables) {
            d->Draw(rotate(mat4(1), radians(-90.f), vec3(0.f, 1.f, 0.f)), view, proj, overrideShader);
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
            ImGui::SliderFloat("Penumbra size", &penumbraSize, 1.f, 2000.f, "%.2f");
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

void Graphics::Init(uvec2 framebufferSize) {
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

void Graphics::OnResize(uvec2 framebufferSize) {
    cc->framebufferSize = framebufferSize;
    cc->cameraProj = perspective(
        radians(45.f), 
        (float)framebufferSize.x / (float)framebufferSize.y, 
        1.f, 
        100.f
    );

    // resize renderbuffer color attachment
    cc->renderTexture->Resize(framebufferSize);

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
        radians(cc->lightInnerCutoffDegrees) * 2.f,
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
    cc->renderTexture->Bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // GUI
    cc->DrawGUI(deltaTime);
}
