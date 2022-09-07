#pragma once
#include <filesystem>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <map>
#include <string>
#include <vector>

#include "Mesh.hpp"

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
    void Activate();
    void SetupVertexAttribs(const VertexAttribInfoList& vertexAttribs);
    void ConnectUniforms(const std::vector<std::string>& uniformNames);
    void InitTextures(const std::vector<TexSettings>& textures);
    
    GLuint Get() {
        return program;
    }

    void SetUniform(const std::string& name, const float& value) {
        Activate();

        glUniform1fv(uniforms[name], 1, &value);
    }
    void SetUniform(const std::string& name, const glm::vec3& value) {
        Activate();

        glUniform3fv(uniforms[name], 1, value_ptr(value));
    }

    void BindTextures();


private:

    static void ReadShaderFile(GLuint shader, GLsizei count, const std::filesystem::path& path);
    static void CompileShader(GLuint shader);

    std::map<std::string, GLint> uniforms;
    std::vector<GLuint> textures;

    GLuint program;

    static GLuint activeProgram;
};
