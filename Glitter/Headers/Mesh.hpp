#pragma once

#include <algorithm>
#include <filesystem>

class Mesh {
public:
    Mesh(const std::filesystem::path& filename);

    unsigned int GetNumElements() {
        return numElements;
    }

    void* GetVertexData() {
        return vertices.data();
    }
    size_t GetVertexDataSize() {
        return sizeof(float) * vertices.size() * componentsPerVertex;
    }

    void* GetIndices() {
        return indices.data();
    }
    size_t GetIndiciesSize() {
        return sizeof(unsigned int) * indices.size();
    }

private:
    static const size_t componentsPerVertex = 14;
    unsigned int numElements;
    std::vector<std::array<float, componentsPerVertex>> vertices;
    std::vector<unsigned int> indices;
};