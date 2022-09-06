#pragma once

#include "VectorMesh.hpp"

class PlanePrimitiveMesh : public VectorMesh {
public:
    PlanePrimitiveMesh(const float size); 

    unsigned int GetNumElements() const {
        return numElements;
    }

private:
    static const unsigned int numElements = 6;
};
