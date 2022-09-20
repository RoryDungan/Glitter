#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

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

    Texture2D(glm::uvec2 size, Format format, Type type, void* data = nullptr)
        : format(format), type(type) {
        glGenTextures(1, &texture);

        Bind();
        InitTexture(size, data);
    }

    virtual ~Texture2D() {
        glDeleteTextures(1, &texture);
    }

    void Resize(glm::ivec2 newSize) {
        Bind();

        InitTexture(newSize, nullptr);
    }

    void SetWrapMode(Wrapping wrapMode) {
        GLint mode = 0;
        switch (wrapMode) {
        case ClampToBorder:
            mode = GL_CLAMP_TO_BORDER;
            break;
        case ClampToEdge:
            mode = GL_CLAMP_TO_EDGE;
            break;
        }

        Bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode);
    }

    void SetFiltering(Filter filter) {
        GLint mode = 0;
        switch (filter) {
        case Nearest:
            mode = GL_NEAREST;
            break;
        case Linear:
            mode = GL_LINEAR;
            break;
        }

        Bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mode);
    }

    void SetBorder(glm::vec4 color) {
        Bind();
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(color));
    }

    GLuint Get() {
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
    }

    void InitTexture(glm::uvec2 size, void* data) {
        glTexImage2D(
            GL_TEXTURE_2D, 
            0, 
            GetGLFormat(),
            size.x, 
            size.y, 
            0, 
            GetGLFormat(), 
            GetGLType(),
            data
        );
    }
};

