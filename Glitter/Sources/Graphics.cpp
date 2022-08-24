#include <iostream>
#include <glm/glm.hpp>
#include "Graphics.hpp"
#include "stb_image.h"
#include "imgui.h"

using namespace glm;

void Graphics::Init() {

    try {
        float vertices[] = {
            //  Position      Color             Texcoords
                -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // Top-left
                 0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // Top-right
                 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // Bottom-right
                -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f  // Bottom-left
        };

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vbo); // Generate 1 buffer
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // Create an element array
        GLuint ebo;
        glGenBuffers(1, &ebo);

        GLuint elements[] = {
            0, 1, 2,
            2, 3, 0
        };

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

        shaderProgram = std::make_unique<Shader>();
        shaderProgram->AttachShader("drawing.vert");
        shaderProgram->AttachShader("drawing.frag");

        glBindFragDataLocation(shaderProgram->Get(), 0, "outColor");

        shaderProgram->Link();
        shaderProgram->Activate();

        GLint posAttrib = glGetAttribLocation(shaderProgram->Get(), "position");
        glEnableVertexAttribArray(posAttrib);
        glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), 0);

        GLint colAttrib = glGetAttribLocation(shaderProgram->Get(), "color");
        glEnableVertexAttribArray(colAttrib);
        glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(2 * sizeof(float)));

        GLint texcoordAttrib = glGetAttribLocation(shaderProgram->Get(), "texcoord");
        glEnableVertexAttribArray(texcoordAttrib);
        glVertexAttribPointer(texcoordAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(5 * sizeof(float)));

        GLuint tex;
        glGenTextures(1, &tex);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);

        int imgWidth, imgHeight, imgBitsPerPixel;
        auto* textureData = stbi_load("vinny.jpg", &imgWidth, &imgHeight, &imgBitsPerPixel, 0);
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

        auto proj = perspective(radians(45.f), 800.f / 600.f, 1.f, 10.f);
        auto uniProj = glGetUniformLocation(shaderProgram->Get(), "proj");
        glUniformMatrix4fv(uniProj, 1, GL_FALSE, value_ptr(proj));

        startTime, lastFrameTime = std::chrono::high_resolution_clock::now();
    }
    catch (std::runtime_error ex) {
        error = ex.what();
        std::cerr << ex.what() << std::endl;
    }
}

void Graphics::Draw() {
    // Background Fill Color
    glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

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


    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    ImGui::Begin("FPS");
    ImGui::Text("%.2f ms\n%.2f FPS", deltaTime * 1000.0f, 1.0f / deltaTime);
    ImGui::End();

    lastFrameTime = std::chrono::high_resolution_clock::now();
}

Graphics::~Graphics() {
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}
