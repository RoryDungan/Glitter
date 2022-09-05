#include <fstream>
#include <sstream>
#include <stb_image.h>

#include "Mesh.hpp"
#include "Shader.hpp"

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

void Shader::ConnectUniforms(const std::vector<std::string>& uniformNames) {
    Activate();

    for (auto& name : uniformNames) {
         uniforms[name] = glGetUniformLocation(program, name.c_str());
    }
}

void Shader::BindTextures(const std::vector<TexSettings>& textureSettings) {
    Activate();

    std::vector<GLuint> textures(textureSettings.size());
    glGenTextures(textureSettings.size(), textures.data());
    
    for (size_t i = 0; i < textureSettings.size(); ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        
        int imgWidth, imgHeight, imgBitsPerPixel;
        auto* textureData = stbi_load(
            textureSettings[i].path.string().c_str(),
            &imgWidth,
            &imgHeight,
            &imgBitsPerPixel,
            0
        );
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imgWidth, imgHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);
        stbi_image_free(textureData);
        
        glUniform1i(glGetUniformLocation(program, textureSettings[i].uniformName.c_str()), i);
        
        for (auto& param : textureSettings[i].params) {
            glTexParameteri(GL_TEXTURE_2D, param.first, param.second);
        }
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
