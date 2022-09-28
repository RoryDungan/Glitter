#version 330 core

struct Material {
    sampler2D normal;
};

in VS_OUT {
    vec3 FragPos;
    vec2 Texcoord;
    mat3 TBN;
    vec4 FragPosLightSpace;
} fs_in;

out vec4 FragColor;

uniform vec3 worldSpaceCameraPos;
uniform samplerCube cubemap;

uniform Material material;

void main() {
    vec3 normalMapSample = texture(material.normal, fs_in.Texcoord).rgb;
    normalMapSample = normalMapSample * 2.0 - 1; // Convert from 0..1 to -1..1
    vec3 normal = normalize(fs_in.TBN * normalMapSample); // transform from tangent to world space

    vec3 I = normalize(fs_in.FragPos - worldSpaceCameraPos);
    vec3 R = reflect(I, normal);
    FragColor = vec4(texture(cubemap, R).rgb, 1.0);
}

