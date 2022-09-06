#include "PlanePrimitiveMesh.hpp"

PlanePrimitiveMesh::PlanePrimitiveMesh(float size) {
    vertices = {
        { -size / 2.f, 0.f, -size / 2.f, 0.f, 1.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, -1.f, 0.f, 0.f },
        { -size / 2.f, 0.f,  size / 2.f, 0.f, 1.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, -1.f, 0.f, 1.f },
        {  size / 2.f, 0.f,  size / 2.f, 0.f, 1.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, -1.f, 1.f, 1.f },
        {  size / 2.f, 0.f, -size / 2.f, 0.f, 1.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, -1.f, 1.f, 0.f },
    };

    indices = { 0, 1, 2, 2, 3, 0 };
}
