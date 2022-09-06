#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <array>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <sstream>
#include <stdexcept>

#include "FileMesh.hpp"

template<std::size_t DESTOFFSET, std::size_t DESTSIZE>
static void staticCopyToVector(const aiVector3t<float>& vert, std::array<float, DESTSIZE>& dstVector) {
    static_assert(DESTOFFSET + 3 <= DESTSIZE);
    dstVector[DESTOFFSET] = vert.x;
    dstVector[DESTOFFSET + 1] = vert.y;
    dstVector[DESTOFFSET + 2] = vert.z;
}

FileMesh::FileMesh(const std::filesystem::path& filename) {
    Assimp::Importer importer;

    const auto* scene = importer.ReadFile(filename.string(),
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType);

    if (scene == nullptr) {
        std::ostringstream s;
        s << "Could not load model file \"" << filename << '"';
        throw std::runtime_error(s.str());
    }
    if (scene->mNumMeshes <= 0) {
        std::ostringstream s;
        s << "Could not find any meshes in model file \"" << filename << '"';
        throw std::runtime_error(s.str());
    }

    auto* mesh = scene->mMeshes[0];
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
