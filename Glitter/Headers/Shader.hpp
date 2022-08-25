#pragma once
#include <filesystem>
#include <glad/glad.h>
#include <map>
#include <string>
#include <vector>

struct VertexAttribInfo {
    std::string name;
    GLint size;
    GLenum type;
};

struct TexSettings {
    std::filesystem::path path;
    std::string uniformName;
    std::map<GLenum, GLint> params;
};

class Shader {
public:
    Shader() {
        program = glCreateProgram();
    }

    ~Shader() {
        glDeleteProgram(program);
    }

    void AttachShader(const std::filesystem::path& path);
    void Link();
    void Activate() {
        glUseProgram(program);
    }
    void SetupVertexAttribs(std::vector<VertexAttribInfo> vertexAttribs);
    void BindTextures(std::vector<TexSettings> textures);
    
    GLuint Get() {
        return program;
    }


private:
    static void ReadShaderFile(GLuint shader, GLsizei count, const std::filesystem::path& path);
    static void CompileShader(GLuint shader);

    GLuint program;

};
