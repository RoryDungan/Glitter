#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <array>
#include <chrono>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "FileMesh.hpp"
#include "VectorMesh.hpp"

template<std::size_t DESTOFFSET, std::size_t DESTSIZE>
static void staticCopyToVector(const aiVector3t<float>& vert, std::array<float, DESTSIZE>& dstVector) {
    static_assert(DESTOFFSET + 3 <= DESTSIZE);
    dstVector[DESTOFFSET] = vert.x;
    dstVector[DESTOFFSET + 1] = vert.y;
    dstVector[DESTOFFSET + 2] = vert.z;
}

std::vector<FileMesh> LoadFileMesh(const std::filesystem::path& path) {
    Assimp::Importer importer;

    auto t1 = std::chrono::high_resolution_clock::now();
    const auto* scene = importer.ReadFile(path.string(),
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType);
    auto t2 = std::chrono::high_resolution_clock::now();

    if (scene == nullptr) {
        std::ostringstream s;
        s << "Could not load model file \"" << path << '"';
        throw std::runtime_error(s.str());
    }
    if (scene->mNumMeshes <= 0) {
        std::ostringstream s;
        s << "Could not find any meshes in model file \"" << path << '"';
        throw std::runtime_error(s.str());
    }

    std::vector<FileMesh> meshes;
    for (int i = 0; i < scene->mNumMeshes; ++i) {
        meshes.push_back(FileMesh(scene->mMeshes[i]));
    }
    auto t3 = std::chrono::high_resolution_clock::now();

    auto assimpLoadTime = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
    auto processingTime = std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2);
    auto totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t1);

    std::cout << "Assimp load time: " << assimpLoadTime.count() << "ms" << std::endl;
    std::cout << "processing time: " << processingTime.count() << "ms" << std::endl;
    std::cout << "total time: " << totalTime.count() << "ms" << std::endl;
    return meshes;
}

FileMesh::FileMesh(const aiMesh* mesh) {
    vertices.resize(mesh->mNumVertices);

    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        staticCopyToVector<0>(mesh->mVertices[i], vertices[i]);
        staticCopyToVector<3>(mesh->mNormals[i], vertices[i]);
        staticCopyToVector<6>(mesh->mTangents[i], vertices[i]);
        staticCopyToVector<9>(mesh->mBitangents[i], vertices[i]);
        vertices[i][12] = mesh->mTextureCoords[0][i].x;
        vertices[i][13] = mesh->mTextureCoords[0][i].y;
    }

    indices.resize(mesh->mNumFaces * 3);
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        indices[i * 3] = mesh->mFaces[i].mIndices[0];
        indices[i * 3 + 1] = mesh->mFaces[i].mIndices[1];
        indices[i * 3 + 2] = mesh->mFaces[i].mIndices[2];
    }

    numElements = mesh->mNumFaces * 3;
}
