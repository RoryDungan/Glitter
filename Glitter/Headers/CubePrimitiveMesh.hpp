#pragma once

#include "VectorMesh.hpp"

class CubePrimitiveMesh : public VectorMesh {
public:
    CubePrimitiveMesh(const float size);

    unsigned int GetNumElements() const {
        return numElements;
    }

private:
    static const unsigned int numElements = 36;
};
