#version 330 core

out vec4 FragColor;

in VS_OUT {
    vec2 TexCoords;
} fs_in;

uniform sampler2D screenTexture;
uniform float time;

void main() {
    vec4 debugColor = vec4(fs_in.TexCoords.x, fs_in.TexCoords.y, 1, 1);
    FragColor = mix(texture(screenTexture, fs_in.TexCoords), debugColor, 0.5);
}
