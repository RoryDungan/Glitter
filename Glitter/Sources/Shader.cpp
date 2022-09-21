#include <fstream>
#include <sstream>

#include "Mesh.hpp"
#include "Shader.hpp"

using namespace glm;

struct VertexAttribPointerSettings {
    GLint attrib;
    GLint size;
    GLenum type;
    int pointer;
};

GLuint Shader::activeProgram = 0;

void Shader::AttachShader(const std::filesystem::path& path) {
    auto extension = path.extension();
    GLuint shader;
    if (extension == ".frag") {
        shader = glCreateShader(GL_FRAGMENT_SHADER);
    }
    else if (extension == ".vert") {
        shader = glCreateShader(GL_VERTEX_SHADER);
    }
    else {
        throw std::invalid_argument("invalid shader file type\"" + extension.string() + "\"");
    }

    ReadShaderFile(shader, 1, path);
    CompileShader(shader);
    glAttachShader(program, shader);
    glDeleteShader(shader);
}

void Shader::Link() {
    glLinkProgram(program);
    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
        GLint messageLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &messageLength);
        std::vector<char> buffer(messageLength);
        glGetProgramInfoLog(program, messageLength, nullptr, buffer.data());
        std::ostringstream msg;
        msg << "Error linking program: " << buffer.data();
        throw std::runtime_error(msg.str());
    }

    FindUniforms();
}

void Shader::FindUniforms() {
    GLint count = 0, size = 0, bufSize = 0;
    GLsizei length = 0;
    GLenum type;
    glGetProgramiv(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &bufSize);
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &count);
    GLchar* name = new GLchar[bufSize];
    for (int i = 0; i < count; i++) {
        glGetActiveUniform(program, (GLuint)i, bufSize, &length, &size, &type, name);
        uniforms[std::string(name)] = glGetUniformLocation(program, name);
    }
    delete[] name;
}

void Shader::Activate() {
    if (activeProgram == program) {
        return;
    }
    glUseProgram(program);
    activeProgram = program;
}

void Shader::SetupVertexAttribs(const VertexAttribInfoList& vertexAttribs) {
    std::vector<VertexAttribPointerSettings> vertexAttribPointers(vertexAttribs.size());

    int sizeSoFar = 0;
    for (size_t i = 0; i < vertexAttribs.size(); ++i) {
        auto& vertexAttribInfo = vertexAttribs[i];
        auto name = vertexAttribInfo.name.c_str();
        vertexAttribPointers[i] = {
            glGetAttribLocation(program, name),
            vertexAttribInfo.size,
            vertexAttribInfo.type,
            sizeSoFar
        };
        switch (vertexAttribInfo.type) {
        case GL_FLOAT:
            sizeSoFar += vertexAttribInfo.size * sizeof(float);
            break;
        default:
            throw std::out_of_range("Unknown type");
        }
    }

    for (auto& vertexAttribPtr : vertexAttribPointers) {
        glEnableVertexAttribArray(vertexAttribPtr.attrib);
        glVertexAttribPointer(
            vertexAttribPtr.attrib,
            vertexAttribPtr.size,
            vertexAttribPtr.type,
            GL_FALSE,
            sizeSoFar,
            (void*)vertexAttribPtr.pointer
        );
    }
}

void Shader::SetUniform(const std::string& name, int value) {
    Activate();

    auto uniformLocation = uniforms.find(name);
    if (uniformLocation != uniforms.end()) {
        glUniform1i(uniformLocation->second, value);
    }
}

void Shader::SetUniform(const std::string& name, float value) {
    Activate();

    auto uniformLocation = uniforms.find(name);
    if (uniformLocation != uniforms.end()) {
        glUniform1f(uniformLocation->second, value);
    }
}

void Shader::SetUniform(const std::string& name, const vec3& value) {
    Activate();

    auto uniformLocation = uniforms.find(name);
    if (uniformLocation != uniforms.end()) {
        glUniform3fv(uniformLocation->second, 1, value_ptr(value));
    }
}

void Shader::SetUniform(const std::string& name, const vec4& value) {
    Activate();

    auto uniformLocation = uniforms.find(name);
    if (uniformLocation != uniforms.end()) {
        glUniform4fv(uniformLocation->second, 1, value_ptr(value));
    }
}

void Shader::SetUniform(const std::string& name, const mat4& value) {
    Activate();

    auto uniformLocation = uniforms.find(name);
    if (uniformLocation != uniforms.end()) {
        glUniformMatrix4fv(uniformLocation->second, 1, GL_FALSE, value_ptr(value));
    }
}

void Shader::AddTexture(const std::string& uniformName, std::shared_ptr<Texture2D> texture) {
    int textureUnit = (int)textures.size();
    textures.push_back(texture);
    
    SetUniform(uniformName, textureUnit);
}

void Shader::BindTextures() {
    for (size_t i = 0; i < textures.size(); ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        textures[i]->Bind();
    }
}

void Shader::ReadShaderFile(GLuint shader, GLsizei count, const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("Could not read file \"" + path.string() + "\"");
    }
    std::stringstream sstr;
    sstr << file.rdbuf();
    auto file_contents = sstr.str();
    const char* shaderSource = file_contents.c_str();

    glShaderSource(shader, count, &shaderSource, NULL);
}

void Shader::CompileShader(GLuint shader) {
    glCompileShader(shader);
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        GLint messageLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &messageLength);
        std::vector<char> buffer(messageLength);
        glGetShaderInfoLog(shader, messageLength, NULL, buffer.data());
        std::ostringstream msg;
        msg << "Error compiling shader: " << buffer.data();
        throw std::runtime_error(msg.str());
    }
}
