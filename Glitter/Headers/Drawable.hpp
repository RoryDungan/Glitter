#pragma once

#include "Mesh.hpp"

class Shader;

class Drawable {
public:
    Drawable(Mesh mesh, Shader shader);

    ~Drawable();

    void Draw();
};