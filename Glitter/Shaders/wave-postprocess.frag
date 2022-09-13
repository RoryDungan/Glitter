#version 330 core

out vec4 FragColor;

in VS_OUT {
    vec2 TexCoords;
} fs_in;

uniform sampler2D screenTexture;
uniform float time;

void main() {
    float displacement = sin(fs_in.TexCoords.x * 80 + time) * 0.01;
    FragColor = texture(screenTexture, vec2(fs_in.TexCoords.x, fs_in.TexCoords.y + displacement));
}
