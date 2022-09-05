#pragma once

#include <array>
#include <filesystem>
#include <vector>

#include "Mesh.hpp"

class FileMesh : public Mesh {
public:
    FileMesh(const std::filesystem::path& filename);

    const unsigned int GetNumElements() const {
        return numElements;
    }

    const void* const GetVertexData() const {
        return vertices.data();
    }
    const size_t GetVertexDataSize() const {
        return sizeof(float) * vertices.size() * componentsPerVertex;
    }

    const void* const GetIndices() const {
        return indices.data();
    }
    const size_t GetIndiciesSize() const {
        return sizeof(unsigned int) * indices.size();
    }

    const VertexAttribInfoList& GetVertexAttribs() const {
        return VertexAttribs;
    }

private:
    static const size_t componentsPerVertex = 14;
    static const VertexAttribInfoList VertexAttribs;

    unsigned int numElements;
    std::vector<std::array<float, componentsPerVertex>> vertices;
    std::vector<unsigned int> indices;

};