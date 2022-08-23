#pragma once
#include <filesystem>
#include <glad/glad.h>
#include <stdexcept>

class Shader {
public:
    Shader() {
        program = glCreateProgram();
    }

    ~Shader() {
        glDeleteProgram(program);
    }

    void AttachShader(const std::filesystem::path& path);
    
    GLuint Get() {
        return program;
    }

    void Link();

    void Activate() {
        glUseProgram(program);
    }

private:
    static void ReadShaderFile(GLuint shader, GLsizei count, const std::filesystem::path& path);
    static void CompileShader(GLuint shader);

    GLuint program;
};
