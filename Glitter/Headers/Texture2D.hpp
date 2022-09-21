#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <filesystem>

class Texture2D {
public:
    enum Format {
        RGB,
        RGBA,
        DepthComponent,
    };

    enum Type {
        UnsignedByte,
        Float
    };

    enum Wrapping {
        ClampToBorder,
        ClampToEdge
    };

    enum Filter {
        Nearest,
        Linear
    };

    Texture2D(const std::filesystem::path& file);
    Texture2D(const glm::uvec2& size, Format format, Type type, const void* data = nullptr);

    virtual ~Texture2D() {
        glDeleteTextures(1, &texture);
    }

    void Resize(const glm::uvec2& newSize);

    void SetWrapMode(Wrapping wrapMode);

    void SetFiltering(Filter filter);

    void SetBorder(const glm::vec4& color);

    GLuint Get() const {
        return texture;
    }

    void Bind() {
        if (boundTexture != texture) {
            glBindTexture(GL_TEXTURE_2D, texture);
            boundTexture = texture;
        }
    }


private:
    GLuint texture;
    static GLuint boundTexture;

    Type type;
    Format format;

    GLenum GetGLType() {
        switch (type) {
        case UnsignedByte:
            return GL_UNSIGNED_BYTE;
        case Float:
            return GL_FLOAT;
        }
        throw std::runtime_error("Invalid texture type");
    }

    GLint GetGLFormat() {
        switch (format) {
        case RGB:
            return GL_RGB;
        case RGBA:
            return GL_RGBA;
        case DepthComponent:
            return GL_DEPTH_COMPONENT;
        }
        throw std::runtime_error("Invalid format");
    }

    void InitTexture(const glm::uvec2& size, const void* data);
};

