#version 150 core

in vec3 position;
in vec3 normal;
in vec3 tangent;
in vec3 bitangent;
in vec2 texcoord;

out mat3 TBN;
out vec2 Texcoord;

uniform mat4 model;
uniform mat4 modelViewProjection;
uniform mat3 modelInverseTranspose;

void main()
{
    vec3 T = normalize(vec3(model * vec4(tangent,   0.0)));
    vec3 B = normalize(vec3(model * vec4(bitangent, 0.0)));
    vec3 N = normalize(vec3(model * vec4(normal,    0.0)));
    /*
    vec3 T = normalize(modelInverseTranspose * tangent);
    vec3 B = normalize(modelInverseTranspose * bitangent);
    vec3 N = normalize(modelInverseTranspose * normal);
    */
    TBN = mat3(T, B, N);

    Texcoord = texcoord;

    gl_Position = modelViewProjection * vec4(position, 1.0);
}
