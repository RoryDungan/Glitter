#pragma once

#include <array>
#include <vector>

#include "Mesh.hpp"

class VectorMesh : public Mesh {
public:
    const void* GetVertexData() const {
        return vertices.data();
    }
    size_t GetVertexDataSize() const {
        return sizeof(float) * vertices.size() * componentsPerVertex;
    }

    const void* GetIndices() const {
        return indices.data();
    }
    size_t GetIndicesSize() const {
        return sizeof(unsigned int) * indices.size();
    }

    const VertexAttribInfoList& GetVertexAttribs() const {
        return VertexAttribs;
    }

protected:
    static const size_t componentsPerVertex = 14;
    static const VertexAttribInfoList VertexAttribs;

    std::vector<std::array<float, componentsPerVertex>> vertices;
    std::vector<unsigned int> indices;

};
