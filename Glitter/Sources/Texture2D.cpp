#include <functional>
#include <sstream>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Texture2D.hpp"

using namespace glm;

GLuint Texture2D::boundTexture = 0;

ImagePtr LoadTexture(const std::filesystem::path& path, bool flipVertically) {
    auto pathStr = path.string();
    auto* img = new Image();
    stbi_set_flip_vertically_on_load(flipVertically);
    img->data = stbi_load(
        pathStr.c_str(),
        &img->width,
        &img->height,
        &img->channels,
        0
    );
    return ImagePtr(
        img, 
        [](Image* img) { stbi_image_free(img->data); }
    );
}

Texture2D::Texture2D(const uvec2& size, Format format, Type type, const void* data)
    : target(GL_TEXTURE_2D), type(type), format(format) {
    glGenTextures(1, &texture);
    hasTexture = true;

    Bind();
    InitTexture(size, data);
}

Texture2D::Texture2D(const std::filesystem::path& path)
    : target(GL_TEXTURE_2D) {
    auto img = LoadTexture(path, true);

    if (img->data == nullptr) {
        std::ostringstream ss;
        ss << "Couldn't load image file " << path;
        throw std::runtime_error(ss.str());
    }

    glGenTextures(1, &texture);
    hasTexture = true;

    if (img->channels == 3) {
        format = Texture2D::RGB;
    }
    else if (img->channels == 4) {
        format = Texture2D::RGBA;
    }
    type = Texture2D::UnsignedByte;

    Bind();
    InitTexture(uvec2(img->width, img->height), img->data);
}

Texture2D::Texture2D(const std::array<std::filesystem::path, 6>& cubemapFaces)
    : target(GL_TEXTURE_CUBE_MAP), type(Texture2D::UnsignedByte), format(Texture2D::RGB) {
    glGenTextures(1, &texture);
    hasTexture = true;

    Bind();
    for (size_t i = 0; i < cubemapFaces.size(); ++i) {
        auto img = LoadTexture(cubemapFaces[i], false);
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            0,
            GL_RGB,
            img->width,
            img->height,
            0,
            GL_RGB,
            GL_UNSIGNED_BYTE,
            img->data
        );
    }
}

void Texture2D::Resize(const uvec2& newSize) {
    Bind();

    InitTexture(newSize, nullptr);
}

void Texture2D::SetWrapMode(Wrapping wrapMode) {
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
    glTexParameteri(target, GL_TEXTURE_WRAP_S, mode);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, mode);
}

void Texture2D::SetFiltering(Filter filter) {
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
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, mode);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, mode);
}

void Texture2D::SetBorder(const vec4& color) {
    Bind();
    glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, value_ptr(color));
}

void Texture2D::InitTexture(const uvec2& size, const void* data) {
    glTexImage2D(
        target, 
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

