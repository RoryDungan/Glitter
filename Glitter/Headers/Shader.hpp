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
    void ConnectDepthTex(GLuint texid);
    
    GLuint Get() {
        return program;
    }

    void SetUniform(const std::string& name, int value);
    void SetUniform(const std::string& name, float value);
    void SetUniform(const std::string& name, const glm::vec3& value);
    void SetUniform(const std::string& name, const glm::vec4& value);
    void SetUniform(const std::string& name, const glm::mat4& value);

    void BindTextures();


private:

    static void ReadShaderFile(GLuint shader, GLsizei count, const std::filesystem::path& path);
    static void CompileShader(GLuint shader);

    std::map<std::string, GLint> uniforms;
    std::vector<GLuint> textures;

    GLuint program;

    static GLuint activeProgram;
};
