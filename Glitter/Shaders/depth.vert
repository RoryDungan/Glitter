#version 330 core

layout (location = 0) in vec3 position;

uniform mat4 modelViewProjection;

void main()
{
    gl_Position = modelViewProjection * vec4(position, 1.0);
}
