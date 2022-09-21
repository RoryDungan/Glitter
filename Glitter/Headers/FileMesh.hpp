#pragma once

#include <filesystem>

#include "VectorMesh.hpp"

class FileMesh : public VectorMesh {
public:
    FileMesh(const std::filesystem::path& filename, int index = 0);

    unsigned int GetNumElements() const {
        return numElements;
    }

private:
    unsigned int numElements;
};
