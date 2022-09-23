#pragma once

#include <filesystem>
#include "VectorMesh.hpp"

class aiMesh;

class FileMesh : public VectorMesh {
public:
    FileMesh(const aiMesh* mesh);

    unsigned int GetNumElements() const {
        return numElements;
    }

private:
    unsigned int numElements;
};

std::vector<FileMesh> LoadFileMesh(const std::filesystem::path& path);
