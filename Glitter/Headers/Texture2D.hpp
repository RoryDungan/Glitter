#pragma once

#include <array>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <filesystem>
#include <functional>
#include <memory>

struct Image {
    int width = 0, height = 0, channels = 0;
    unsigned char* data = nullptr;
};

typedef std::unique_ptr<Image, void(*)(Image*)> ImagePtr;
ImagePtr LoadTexture(const std::filesystem::path& path, bool flipVertically);

//void LoadTexture(
//    const std::filesystem::path& path, 
//    std::function<void(const unsigned char*, int, int, int)> callback
//);

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

    enum ColorSpace {
        LinearSpace,
        sRGB
    };

    Texture2D(const std::filesystem::path& file, ColorSpace colorSpace);
    Texture2D(const std::array<std::filesystem::path, 6>& cubemapFaces, ColorSpace colorSpace);
    Texture2D(const glm::uvec2& size, ColorSpace colorSpace, Format format, Type type, const void* data = nullptr);

    virtual ~Texture2D() {
        if (hasTexture) {
            glDeleteTextures(1, &texture);
        }
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
            glBindTexture(target, texture);
            boundTexture = texture;
        }
    }


private:
    GLenum target;
    GLuint texture;
    bool hasTexture = false;
    static GLuint boundTexture;

    Type type;
    Format format;
    ColorSpace colorSpace;

    GLenum GetGLType() const {
        switch (type) {
        case UnsignedByte:
            return GL_UNSIGNED_BYTE;
        case Float:
            return GL_FLOAT;
        }
        throw std::runtime_error("Invalid texture type");
    }

    GLint GetGLInternalFormat() const {
        switch (format) {
            case RGB:
                if (colorSpace == LinearSpace) {
                    return GL_RGB;
                }
                else if (colorSpace == sRGB) {
                    return GL_SRGB;
                }
                break;
            case RGBA:
                if (colorSpace == LinearSpace) {
                    return GL_RGBA;
                }
                else if (colorSpace == sRGB) {
                    return GL_SRGB_ALPHA;
                }
                break;
            case DepthComponent:
                return GL_DEPTH_COMPONENT;
        }
        throw std::runtime_error("Invalid format");
    }

    GLint GetGLFormat() const {
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

