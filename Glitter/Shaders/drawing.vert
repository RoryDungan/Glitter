#version 330 core

in vec3 position;
in vec3 normal;
in vec3 tangent;
in vec3 bitangent;
in vec2 texcoord;

out vec3 FragPos;
out vec2 Texcoord;
out mat3 TBN;

uniform mat4 model;
uniform mat4 modelViewProjection;
uniform mat3 modelInverseTranspose;

void main()
{
    FragPos = (model * vec4(position, 1.0)).xyz;

    Texcoord = texcoord;

    vec3 T = normalize(modelInverseTranspose * tangent);
    vec3 B = normalize(modelInverseTranspose * bitangent);
    vec3 N = normalize(modelInverseTranspose * normal);
    TBN = mat3(T, B, N);

    gl_Position = modelViewProjection * vec4(position, 1.0);
}
