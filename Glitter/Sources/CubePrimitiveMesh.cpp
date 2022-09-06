#include "CubePrimitiveMesh.hpp"

CubePrimitiveMesh::CubePrimitiveMesh(float size) {
    const auto radius = size / 2.f;
    vertices = {
        // top
        { -radius,  radius, -radius, 0.f, 1.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, -1.f, 0.f, 0.f },
        { -radius,  radius,  radius, 0.f, 1.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, -1.f, 0.f, 1.f },
        {  radius,  radius,  radius, 0.f, 1.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, -1.f, 1.f, 1.f },
        {  radius,  radius, -radius, 0.f, 1.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, -1.f, 1.f, 0.f },

        // front
        { -radius,  radius,  radius, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f },
        { -radius, -radius,  radius, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f },
        {  radius, -radius,  radius, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f, 1.f },
        {  radius,  radius,  radius, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f, 0.f },

        // left
        { -radius,  radius, -radius, -1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f, 0.f, 0.f, 0.f },
        { -radius, -radius, -radius, -1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f, 0.f, 0.f, 1.f },
        { -radius, -radius,  radius, -1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f, 0.f, 1.f, 1.f },
        { -radius,  radius,  radius, -1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f, 0.f, 1.f, 0.f },

        // back
        {  radius,  radius, -radius, 0.f, 0.f, -1.f, -1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f },
        {  radius, -radius, -radius, 0.f, 0.f, -1.f, -1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f },
        { -radius, -radius, -radius, 0.f, 0.f, -1.f, -1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f, 1.f },
        { -radius,  radius, -radius, 0.f, 0.f, -1.f, -1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f, 0.f },

        // right
        {  radius,  radius,  radius, 1.f, 0.f, 0.f, 0.f, 0.f, -1.f, 0.f, 1.f, 0.f, 0.f, 0.f },
        {  radius, -radius,  radius, 1.f, 0.f, 0.f, 0.f, 0.f, -1.f, 0.f, 1.f, 0.f, 0.f, 1.f },
        {  radius, -radius, -radius, 1.f, 0.f, 0.f, 0.f, 0.f, -1.f, 0.f, 1.f, 0.f, 1.f, 1.f },
        {  radius,  radius, -radius, 1.f, 0.f, 0.f, 0.f, 0.f, -1.f, 0.f, 1.f, 0.f, 1.f, 0.f },

        // bottom
        { -radius, -radius,  radius, 0.f, -1.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f },
        { -radius, -radius, -radius, 0.f, -1.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f },
        {  radius, -radius, -radius, 0.f, -1.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f },
        {  radius, -radius,  radius, 0.f, -1.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 1.f, 0.f },
    };

    indices = { 
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        8, 9, 10, 10, 11, 8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20,
    };
}
