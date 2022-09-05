#pragma once

#include <glad/glad.h>
#include <string>
#include <vector>

struct VertexAttribInfo {
    std::string name;
    GLint size;
    GLenum type;
};

typedef std::vector<VertexAttribInfo> VertexAttribInfoList;

class Mesh {
public:
    virtual unsigned int GetNumElements() const = 0;

    virtual const void* GetVertexData() const = 0;
    virtual size_t GetVertexDataSize() const = 0;

    virtual const void* GetIndices() const = 0;
    virtual size_t GetIndiciesSize() const = 0;

    virtual const VertexAttribInfoList& GetVertexAttribs() const = 0;
};
