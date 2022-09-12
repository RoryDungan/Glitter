#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float time;

void main() {
    float displacement = sin(TexCoords.x * 80 + time) * 0.01;
    FragColor = texture(screenTexture, vec2(TexCoords.x, TexCoords.y + displacement));
}
