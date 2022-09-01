#pragma once

#include <algorithm>
#include <array>
#include <filesystem>
#include <vector>

#include "Shader.hpp"

class Mesh {
public:
    Mesh(const std::filesystem::path& filename);

    const unsigned int GetNumElements() const {
        return numElements;
    }

    const void* const GetVertexData() const {
        return vertices.data();
    }
    const size_t GetVertexDataSize() const {
        return sizeof(float) * vertices.size() * componentsPerVertex;
    }

    const void* GetIndices() const {
        return indices.data();
    }
    const size_t GetIndiciesSize() const {
        return sizeof(unsigned int) * indices.size();
    }

    static const std::vector<VertexAttribInfo> VertexAttribs;
private:
    static const size_t componentsPerVertex = 14;
    unsigned int numElements;
    std::vector<std::array<float, componentsPerVertex>> vertices;
    std::vector<unsigned int> indices;
};