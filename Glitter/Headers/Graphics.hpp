#pragma once

#include <chrono>

#include <filesystem>
#include <fstream>
#include <sstream>

#include "glitter.hpp"

class Graphics {
public:
    Graphics() { }

    void Init();
    void Draw();

    ~Graphics();
private:
    static bool ReadShaderFile(GLuint shader, GLsizei count, const std::filesystem::path& path);
    static bool CompileShader(GLuint shader);

    std::chrono::steady_clock::time_point startTime;

    GLuint vbo = 0, vao = 0, shaderProgram = 0, vertexShader = 0, fragmentShader = 0;
};