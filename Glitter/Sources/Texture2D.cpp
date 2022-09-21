#include <sstream>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Texture2D.hpp"

using namespace glm;

GLuint Texture2D::boundTexture = 0;

Texture2D::Texture2D(const uvec2& size, Format format, Type type, const void* data)
    : format(format), type(type) {
    glGenTextures(1, &texture);
    hasTexture = true;

    Bind();
    InitTexture(size, data);
}

Texture2D::Texture2D(const std::filesystem::path& path) {
    auto pathStr = path.string();
    int imgWidth, imgHeight, channelsInFile;
    stbi_set_flip_vertically_on_load(true);
    auto* textureData = stbi_load(
        pathStr.c_str(),
        &imgWidth,
        &imgHeight,
        &channelsInFile,
        0
    );
    if (textureData == nullptr) {
        std::ostringstream ss;
        ss << "Couldn't load image file " << path;
        throw std::runtime_error(ss.str());
    }

    glGenTextures(1, &texture);
    hasTexture = true;

    if (channelsInFile == 3) {
        format = Texture2D::RGB;
    }
    else if (channelsInFile == 4) {
        format = Texture2D::RGBA;
    }
    type = Texture2D::UnsignedByte;

    Bind();
    InitTexture(uvec2(imgWidth, imgHeight), textureData);

    stbi_image_free(textureData);
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode);
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mode);
}

void Texture2D::SetBorder(const vec4& color) {
    Bind();
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, value_ptr(color));
}

void Texture2D::InitTexture(const uvec2& size, const void* data) {
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

