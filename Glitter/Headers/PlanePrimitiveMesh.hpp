#pragma once

#include <array>
#include <filesystem>
#include <vector>

#include "Mesh.hpp"

class PlanePrimitiveMesh : public Mesh {
public:
    PlanePrimitiveMesh(const float size) {
        vertices = {
            { -size / 2.f, 0.f, -size / 2.f, 0.f, 1.f, 0.f, 0.f, 0.f, -1.f, 1.f, 0.f, 0.f, 0.f, 0.f },
            {  size / 2.f, 0.f, -size / 2.f, 0.f, 1.f, 0.f, 0.f, 0.f, -1.f, 1.f, 0.f, 0.f, 1.f, 0.f },
            { -size / 2.f, 0.f,  size / 2.f, 0.f, 1.f, 0.f, 0.f, 0.f, -1.f, 1.f, 0.f, 0.f, 0.f, 1.f },
            {  size / 2.f, 0.f,  size / 2.f, 0.f, 1.f, 0.f, 0.f, 0.f, -1.f, 1.f, 0.f, 0.f, 1.f, 1.f },
        };

        indices = { 0, 1, 2, 2, 1, 3 };
    }

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

    static const unsigned int numElements = 6;
    std::vector<std::array<float, componentsPerVertex>> vertices;
    std::vector<unsigned int> indices;

};