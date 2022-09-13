#version 330 core

out vec4 FragColor;

in VS_OUT {
    vec2 TexCoords;
} fs_in;

uniform sampler2D screenTexture;
uniform float time;

const float offset = 1.0 / 400.0;

void main() {
    const int samples = 9;
    vec2 offsets[samples] = vec2[](
        vec2(-offset, offset), vec2(0.0f, offset), vec2(offset, offset),
        vec2(-offset, 0.0f), vec2(0.0f, 0.0f), vec2(offset, 0.0f),
        vec2(-offset, -offset), vec2(0.0f, -offset), vec2(offset, -offset)
    );

    float kernel[samples] = float[](
        1, 1, 1,
        1, -9, 1,
        1, 1, 1
    );


    vec3 sampleTex[samples];
    for (int i = 0; i < samples; ++i) {
        sampleTex[i] = vec3(texture(screenTexture, fs_in.TexCoords.st + offsets[i]));
    }
    vec3 col = vec3(0.0);
    for (int i = 0; i < samples; ++i) {
        col += sampleTex[i] * kernel[i];
    }

    FragColor = vec4(col, 1.0);
}
