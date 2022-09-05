#pragma once

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
    virtual const unsigned int GetNumElements() const = 0;

    virtual const void* const GetVertexData() const = 0;
    virtual const size_t GetVertexDataSize() const = 0;

    virtual const void* const GetIndices() const = 0;
    virtual const size_t GetIndiciesSize() const = 0;

    virtual const VertexAttribInfoList& GetVertexAttribs() const = 0;
};