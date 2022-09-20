#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 tangent;
layout (location = 3) in vec3 bitangent;
layout (location = 4) in vec2 texcoord;

out VS_OUT {
    vec3 FragPos;
    vec2 Texcoord;
    mat3 TBN;
    vec4 FragPosLightSpace;
} vs_out;

uniform mat4 model;
uniform mat4 modelViewProjection;
uniform mat3 modelInverseTranspose;
uniform mat4 lightSpaceMatrix;

void main()
{
    vs_out.FragPos = (model * vec4(position, 1.0)).xyz;

    vs_out.Texcoord = texcoord;

    vec3 T = normalize(modelInverseTranspose * tangent);
    vec3 B = normalize(modelInverseTranspose * bitangent);
    vec3 N = normalize(modelInverseTranspose * normal);
    vs_out.TBN = mat3(T, B, N);

    vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0); 

    gl_Position = modelViewProjection * vec4(position, 1.0);
}
