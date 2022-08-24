#include <iostream>
#include <glm/glm.hpp>
#include "Graphics.hpp"
#include "stb_image.h"
#include "imgui.h"

using namespace glm;

void Graphics::Init(ivec2 windowSize) {
    try {
        glEnable(GL_DEPTH_TEST);

        GLfloat vertices[] = {
            -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
             0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
             0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
             0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
            -0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,

            -0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
             0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
             0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
             0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,

            -0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
            -0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,

             0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
             0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
             0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
             0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
             0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
             0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,

            -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
             0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
             0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
             0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
            -0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,

            -0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
             0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
             0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
             0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
            -0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
            -0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f
        };

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vbo); // Generate 1 buffer
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // Create an element array
        //GLuint ebo;
        //glGenBuffers(1, &ebo);

        //GLuint elements[] = {
        //    0, 1, 2,
        //    2, 3, 0
        //};

        //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

        shaderProgram = std::make_unique<Shader>();
        shaderProgram->AttachShader("drawing.vert");
        shaderProgram->AttachShader("drawing.frag");

        glBindFragDataLocation(shaderProgram->Get(), 0, "outColor");

        shaderProgram->Link();
        shaderProgram->Activate();

        shaderProgram->SetupVertexAttribs({
            {"position", 3, GL_FLOAT},
            {"color", 3, GL_FLOAT},
            {"texcoord", 2, GL_FLOAT},
        });

        GLuint tex;
        glGenTextures(1, &tex);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);

        int imgWidth, imgHeight, imgBitsPerPixel;
        auto* textureData = stbi_load(
            "vinny.jpg", 
            &imgWidth, 
            &imgHeight, 
            &imgBitsPerPixel, 
            0
        );
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imgWidth, imgHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);
        stbi_image_free(textureData);

        glUniform1i(glGetUniformLocation(shaderProgram->Get(), "tex"), 0);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        auto model = mat4(1.f);
        uniModel = glGetUniformLocation(shaderProgram->Get(), "model");
        glUniformMatrix4fv(uniModel, 1, GL_FALSE, value_ptr(model));

        auto view = lookAt(
            vec3(1.2f, 1.2f, 1.2f),
            vec3(0.f, 0.f, 0.f),
            vec3(0.f, 0.f, 1.f)
        );
        auto uniView = glGetUniformLocation(shaderProgram->Get(), "view");
        glUniformMatrix4fv(uniView, 1, GL_FALSE, value_ptr(view));

        uniProj = glGetUniformLocation(shaderProgram->Get(), "proj");
        UpdateAspect(windowSize);

        startTime, lastFrameTime = std::chrono::high_resolution_clock::now();
    }
    catch (std::runtime_error ex) {
        error = ex.what();
        std::cerr << ex.what() << std::endl;
    }
}

void Graphics::UpdateAspect(ivec2 windowSize) {
    auto proj = perspective(
        radians(45.f), 
        (float)windowSize.x / (float)windowSize.y, 
        1.f, 
        10.f
    );
    glUniformMatrix4fv(uniProj, 1, GL_FALSE, value_ptr(proj));
}

void Graphics::Draw() {
    // Background Fill Color
    glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!error.empty()) {
        ImGui::Begin("Error");
        ImGui::Text(error.c_str());
        ImGui::End();
        return;
    }

    auto t_now = std::chrono::high_resolution_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - startTime).count();
    auto deltaTime = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - lastFrameTime).count();

    auto model = mat4(1.0f);
    model = rotate(model, time * radians(180.f), vec3(0.f, 0.f, 1.f));
    glUniformMatrix4fv(uniModel, 1, GL_FALSE, value_ptr(model));


    //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    ImGui::Begin("FPS");
    ImGui::Text("%.2f ms\n%.2f FPS", deltaTime * 1000.0f, 1.0f / deltaTime);
    ImGui::End();

    lastFrameTime = std::chrono::high_resolution_clock::now();
}

Graphics::~Graphics() {
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}
