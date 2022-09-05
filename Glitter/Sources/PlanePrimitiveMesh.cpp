#include <glad/glad.h>

#include "PlanePrimitiveMesh.hpp"

const VertexAttribInfoList PlanePrimitiveMesh::VertexAttribs = {
    {"position", 3, GL_FLOAT},
    {"normal", 3, GL_FLOAT},
    {"tangent", 3, GL_FLOAT},
    {"bitangent", 3, GL_FLOAT},
    {"texcoord", 2, GL_FLOAT},
};
