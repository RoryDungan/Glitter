#include "Shader.hpp"
#include <fstream>
#include <sstream>


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
        std::unique_ptr<char[]> buffer(new char[messageLength]);
        glGetProgramInfoLog(program, messageLength, nullptr, buffer.get());
        std::ostringstream msg;
        msg << "Error linking program: " << buffer.get();
        throw std::runtime_error(msg.str());
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
        std::unique_ptr<char[]> buffer(new char[messageLength]);
        glGetShaderInfoLog(shader, messageLength, NULL, buffer.get());
        std::ostringstream msg;
        msg << "Error compiling shader: " << buffer.get();
        throw std::runtime_error(msg.str());
    }
}
