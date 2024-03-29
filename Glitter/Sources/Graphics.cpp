#include <algorithm>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <iostream>
#include <stdexcept>
#include <stb_image.h>
#include <iostream>

#include "ArcCamera.hpp"
#include "CubePrimitiveMesh.hpp"
#include "Drawable.hpp"
#include "Graphics.hpp"
#include "FileMesh.hpp"
#include "PlanePrimitiveMesh.hpp"
#include "Shader.hpp"
#include "Texture2D.hpp"
#include "Timer.hpp"

using namespace glm;

void PrintMatrix(char* name, mat4 mat) {
    printf("%s matrix:\n", name);
    printf("%.2f, %.2f, %.2f, %.2f\n", mat[0].x, mat[1].x, mat[2].x, mat[3].x);
    printf("%.2f, %.2f, %.2f, %.2f\n", mat[0].y, mat[1].y, mat[2].y, mat[3].y);
    printf("%.2f, %.2f, %.2f, %.2f\n", mat[0].z, mat[1].z, mat[2].z, mat[3].z);
    printf("%.2f, %.2f, %.2f, %.2f\n", mat[0].w, mat[1].w, mat[2].w, mat[3].w);
}

const float skyboxVertices[] = {
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

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

static const unsigned int SHADOW_WIDTH = 512, SHADOW_HEIGHT = 512;

static const float kCameraDistance = 2.f;
static const float kMouseSensitivity = 0.01f;
static const float kAmbientFactor = 0.2f;

static const ImGuiColorEditFlags ColorEditFlags = ImGuiColorEditFlags_PickerHueWheel;

struct Graphics::CheshireCat {
    std::unique_ptr<Timer> timer;
    std::vector<std::shared_ptr<Drawable>> characterDrawables;
    std::unique_ptr<Drawable> floor;
    std::unique_ptr<Drawable> pointLightDrawable;

    std::shared_ptr<Shader> pointLightShader;
    std::vector<std::shared_ptr<Shader>> sceneShaders;
    std::shared_ptr<Shader> floorShader;

    vec3 cameraCentre = vec3(0.f, 1.4f, 0.f);
    mat4 cameraView = mat4(), cameraProj = mat4();
    ArcCamera camera;
    dvec2 cursorPosition = dvec2(0.0, 0.0);
    bool mouseClicked = false;

    ivec2 framebufferSize = ivec2(0, 0);

    vec3 lightStartPos = vec3(2.2f, 4.f, 2.f);
    float lightInnerCutoffDegrees = 35.f;
    float lightEdgeRadiusDegrees = 12.f;
    Light light = {
        .position = lightStartPos,
        .direction = normalize(vec3(0.f) - lightStartPos), // look towards the center of the scene
        .cutOff = cos(radians(lightInnerCutoffDegrees)),
        .outerCutOff = cos(radians(lightInnerCutoffDegrees + lightEdgeRadiusDegrees)),

        .ambient = vec3(kAmbientFactor),
        .diffuse = vec3(1.f),
        .specular = vec3(1.f),

        //1.f, 0.09f, 0.032f // distance 50
        .constant = 1.f,
        .linear = 0.027f,
        .quadratic = 0.0028f // distance 160
    };
    mat4 lightMat = mat4(1), lightSpaceMatrix = mat4(1);

    GLuint depthMapFBO = 0;
    std::shared_ptr<Texture2D> depthMap;
    std::shared_ptr<Shader> depthShader;
    float penumbraSize = 500.f;

    GLuint fbo = 0, rbo = 0, quadVAO = 0, quadVBO = 0;
    std::shared_ptr<Texture2D> renderTexture = 0;

    std::unique_ptr<Shader> screenShader;

    std::shared_ptr<Texture2D> skyboxTexture;
    std::unique_ptr<Shader> skyboxShader;
    GLuint skyboxVAO = 0, skyboxVBO = 0;

    std::string error;

    CheshireCat() 
        : camera(cameraCentre, kCameraDistance, 0.2f, 0.2f) {
    }

    ~CheshireCat() {
        if (depthMapFBO != 0) {
            glDeleteFramebuffers(1, &depthMapFBO);
        }
        if (fbo != 0) {
            glDeleteFramebuffers(1, &fbo);
        }
        if (rbo != 0) {
            glDeleteRenderbuffers(1, &rbo);
        }
        if (quadVAO != 0) {
            glDeleteBuffers(1, &quadVAO);
        }
        if (quadVBO != 0) {
            glDeleteBuffers(1, &quadVBO);
        }
        if (skyboxVBO != 0) {
            glDeleteBuffers(1, &skyboxVBO);
        }
        if (skyboxVAO != 0) {
            glDeleteVertexArrays(1, &skyboxVAO);
        }
    }

    void InitSkybox() {
        glGenVertexArrays(1, &skyboxVAO);
        glGenBuffers(1, &skyboxVBO);
        glBindVertexArray(skyboxVAO);
        glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

        std::filesystem::path skyboxDir = "skybox";
        std::array<std::filesystem::path, 6> textureFaces = {
            skyboxDir / "right.jpg",
            skyboxDir / "left.jpg",
            skyboxDir / "top.jpg",
            skyboxDir / "bottom.jpg",
            skyboxDir / "front.jpg",
            skyboxDir / "back.jpg",
        };
        skyboxTexture = std::make_shared<Texture2D>(textureFaces, Texture2D::sRGB);
        skyboxTexture->SetFiltering(Texture2D::Linear);
        skyboxTexture->SetWrapMode(Texture2D::ClampToEdge);

        skyboxShader = std::make_unique<Shader>();
        skyboxShader->AttachShader("skybox.vert");
        skyboxShader->AttachShader("skybox.frag");
        skyboxShader->Link();
        skyboxShader->AddTexture("cubemap", skyboxTexture);
    }

    void InitDepthBuffer() {
        // Setup depth map
        glGenFramebuffers(1, &depthMapFBO);

        depthMap = std::make_shared<Texture2D>(
            uvec2(SHADOW_WIDTH, SHADOW_HEIGHT),
            Texture2D::LinearSpace,
            Texture2D::DepthComponent,
            Texture2D::Float);
        depthMap->SetFiltering(Texture2D::Linear);
        depthMap->SetWrapMode(Texture2D::ClampToBorder);
        depthMap->SetBorder(vec4(1.f, 1.f, 1.f, 1.f));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_GREATER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);


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

        auto characterDiffuse = std::make_shared<Texture2D>("TP_Guide_S0_DF.png", Texture2D::sRGB);
        characterDiffuse->SetWrapMode(Texture2D::ClampToEdge);
        characterDiffuse->SetFiltering(Texture2D::Linear);
        shader->AddTexture("material.diffuse", characterDiffuse);

        auto characterNormalMap = std::make_shared<Texture2D>("TP_Guide_S0_NM.png", Texture2D::LinearSpace);
        characterNormalMap->SetWrapMode(Texture2D::ClampToEdge);
        characterNormalMap->SetFiltering(Texture2D::Linear);
        shader->AddTexture("material.normal", characterNormalMap);

        auto characterSpecular = std::make_shared<Texture2D>("black.png", Texture2D::sRGB);
        characterSpecular->SetWrapMode(Texture2D::ClampToEdge);
        characterSpecular->SetFiltering(Texture2D::Linear);
        shader->AddTexture("material.specular", characterSpecular);

        shader->AddTexture("shadowMap", depthMap);

        auto hairShader = std::make_shared<Shader>();
        hairShader->AttachShader("drawing.vert");
        hairShader->AttachShader("textured.frag");
        hairShader->Link();
        hairShader->SetUniform("material.shininess", 32.f);

        auto hairDiffuse = std::make_shared<Texture2D>("TP_Guide_S0_Hair_DF.png", Texture2D::sRGB);
        hairDiffuse->SetWrapMode(Texture2D::ClampToEdge);
        hairDiffuse->SetFiltering(Texture2D::Linear);
        hairShader->AddTexture("material.diffuse", hairDiffuse);

        auto hairNormalMap = std::make_shared<Texture2D>("TP_Guide_S0_Hair_NM.png", Texture2D::LinearSpace);
        hairNormalMap->SetWrapMode(Texture2D::ClampToEdge);
        hairNormalMap->SetFiltering(Texture2D::Linear);
        hairShader->AddTexture("material.normal", hairNormalMap);

        hairShader->AddTexture("material.specular", characterSpecular);
        hairShader->AddTexture("shadowMap", depthMap);

        auto meshes = LoadFileMesh("Skye.obj");
        characterDrawables = {
            std::make_shared<Drawable>(meshes[0], shader),
            std::make_shared<Drawable>(meshes[1], shader),
            std::make_shared<Drawable>(meshes[2], hairShader),
        };
        sceneShaders.push_back(shader);
        sceneShaders.push_back(hairShader);


        PlanePrimitiveMesh floorMesh(12.f);
        floorShader = std::make_shared<Shader>();
        floorShader->AttachShader("drawing.vert");
        floorShader->AttachShader("textured.frag");
        floorShader->Link();
        floorShader->SetUniform("material.shininess", 32.f);

        auto floorDiffuse = std::make_shared<Texture2D>("brickwall.jpg", Texture2D::sRGB);
        floorDiffuse->SetWrapMode(Texture2D::ClampToEdge);
        floorDiffuse->SetFiltering(Texture2D::Linear);
        floorShader->AddTexture("material.diffuse", floorDiffuse);

        auto floorNormal = std::make_shared<Texture2D>("brickwall_normal.jpg", Texture2D::LinearSpace);
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
        renderTexture = std::make_shared<Texture2D>(framebufferSize, Texture2D::LinearSpace, Texture2D::RGB, Texture2D::UnsignedByte);
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
        cameraView = camera.GetViewMatrix();
        cameraProj = perspective(
            radians(45.f), 
            (float)framebufferSize.x / (float)framebufferSize.y, 
            0.1f, 
            100.f
        );
    }

    void UpdateScene(float time, float deltaTime) {
        // Move light
        auto lightRotation = rotate(
            mat4(1.f),
            time * radians(-20.f), vec3(0, 1, 0)
        );
        lightMat = translate(
            mat4(1.f), // lightRotation,
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

        cameraView = camera.GetViewMatrix();
    }

    void DrawSkybox(mat4 view, mat4 proj) {
        glDepthFunc(GL_LEQUAL);
        skyboxShader->Activate();
        skyboxShader->SetUniform("viewProjection", proj * mat4(mat3(view)));
        skyboxShader->BindTextures();
        glBindVertexArray(skyboxVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDepthFunc(GL_LESS);
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
        light.ambient = light.specular * kAmbientFactor;
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
}

void Graphics::Init(uvec2 framebufferSize, dvec2 cursorPosition) {
    try {
        cc->framebufferSize = framebufferSize;
        cc->cursorPosition = cursorPosition;

        glEnable(GL_CULL_FACE);

        cc->InitDepthBuffer();
        cc->InitSkybox();
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
        0.1f, 
        100.f
    );

    // resize renderbuffer color attachment
    cc->renderTexture->Resize(framebufferSize);

    // resize depth attachment
    glBindRenderbuffer(GL_RENDERBUFFER, cc->rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, framebufferSize.x, framebufferSize.y);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void Graphics::OnCursorMoved(dvec2 cursorPos) {
    if (cc->mouseClicked) {
        auto delta = cursorPos - cc->cursorPosition;
        cc->camera.Yaw(-delta.x * kMouseSensitivity);
        cc->camera.Pitch(delta.y * kMouseSensitivity);
    }
    cc->cursorPosition = cursorPos;
}

void Graphics::OnMouseButton(int button, int action) {
    if (button == 0) {
        if (action == 1) {
            auto& io = ImGui::GetIO();
            if (!io.WantCaptureMouse) {
                cc->mouseClicked = true;
            }
        }
        else if (action == 0) {
            cc->mouseClicked = false;
        }
    }
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

    cc->UpdateScene(time, deltaTime);

    glEnable(GL_DEPTH_TEST);
    // depth pass
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, cc->depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    cc->DrawFirstPass(lightView, lightProjection, time, cc->depthShader);

    // first pass
    glViewport(0, 0, cc->framebufferSize.x, cc->framebufferSize.y);
    glBindFramebuffer(GL_FRAMEBUFFER, cc->fbo);
    glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    cc->DrawFirstPass(cc->cameraView, cc->cameraProj, time);
    cc->DrawSkybox(cc->cameraView, cc->cameraProj);

    // second pass
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_FRAMEBUFFER_SRGB);
    glClearColor(1.f, 1.f, 1.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    cc->screenShader->Activate();
    cc->screenShader->SetUniform("time", time);
    cc->screenShader->SetUniform("clipPos", vec4(0.f, 0.f, 1.f, 1.f));
    glBindVertexArray(cc->quadVAO);
    glActiveTexture(GL_TEXTURE0);
    cc->renderTexture->Bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisable(GL_FRAMEBUFFER_SRGB);

    // GUI
    cc->DrawGUI(deltaTime);
}
