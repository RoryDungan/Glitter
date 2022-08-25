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
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,
             0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,

            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,
             0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,

            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,

             0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
             0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
             0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
             0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,

            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,
             0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f,
             0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
             0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,

            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f
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
            {"normal", 3, GL_FLOAT},
            {"texcoord", 2, GL_FLOAT},
        });

        GLuint textures[2];
        glGenTextures(2, textures);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[0]);
        int imgWidth, imgHeight, imgBitsPerPixel;
        auto* textureData = stbi_load(
            "metal.jpg",
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
        
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textures[1]);
        textureData = stbi_load(
            "metal_norm.jpg",
            &imgWidth,
            &imgHeight,
            &imgBitsPerPixel,
            0
        );
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imgWidth, imgHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);
        stbi_image_free(textureData);
        glUniform1i(glGetUniformLocation(shaderProgram->Get(), "normalMap"), 1);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        modelViewProjectionLocation = 
            glGetUniformLocation(shaderProgram->Get(), "modelViewProjection");
        modelInverseTransposeLocation = 
            glGetUniformLocation(shaderProgram->Get(), "modelInverseTranspose");

        model = mat4(1.f);
        view = lookAt(
            vec3(1.2f, 1.2f, 1.2f),
            vec3(0.f, 0.f, 0.f),
            vec3(0.f, 0.f, 1.f)
        );
        UpdateAspect(windowSize);

        auto modelInverseTranspose = mat3(transpose(inverse(model)));
        glUniformMatrix3fv(modelInverseTransposeLocation, 1, GL_FALSE, value_ptr(modelInverseTranspose));

        // set the light direction.
        auto uniReverseLightDirection = glGetUniformLocation(shaderProgram->Get(), "reverseLightDirection");
        auto lightDir = normalize(vec3(0.5, 0.7, 1));
        glUniform3fv(uniReverseLightDirection, 1, value_ptr(lightDir));


        startTime = lastFrameTime = std::chrono::high_resolution_clock::now();
    }
    catch (std::runtime_error ex) {
        error = ex.what();
        std::cerr << ex.what() << std::endl;
    }
}

void Graphics::UpdateAspect(ivec2 windowSize) {
    proj = perspective(
        radians(45.f), 
        (float)windowSize.x / (float)windowSize.y, 
        1.f, 
        10.f
    );
    auto mvp = proj * view * model;
    glUniformMatrix4fv(modelViewProjectionLocation, 1, GL_FALSE, value_ptr(mvp));
}

void Graphics::Draw() {
    // Background Fill Color
    glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!error.empty()) {
        ImGui::Begin("Error");
        ImGui::Text("%s", error.c_str());
        ImGui::End();
        return;
    }

    using seconds = std::chrono::duration<float>;
    auto t_now = std::chrono::high_resolution_clock::now();
    auto time = seconds(t_now - startTime).count();
    auto deltaTime = seconds(t_now - lastFrameTime).count();

    model = mat4(1.0f);
    model = rotate(model, time * radians(45.f), vec3(0.f, 0.f, 1.f));
    auto mvp = proj * view * model;
    glUniformMatrix4fv(modelViewProjectionLocation, 1, GL_FALSE, value_ptr(mvp));
    auto modelInverseTranspose = mat3(transpose(inverse(model)));
    glUniformMatrix3fv(modelInverseTransposeLocation, 1, GL_FALSE, value_ptr(modelInverseTranspose));
    

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
