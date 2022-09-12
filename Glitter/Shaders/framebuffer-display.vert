#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

uniform vec4 clipPos = vec4(0,0,1,1);

void main() {
    vec2 pos = aPos * clipPos.zw + clipPos.xy;
    gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);
    TexCoords = aTexCoords;
}