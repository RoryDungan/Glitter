#version 150 core

in vec3 position;
in vec3 normal;
in vec2 texcoord;

out vec3 Normal;
out vec2 Texcoord;

uniform mat4 modelViewProjection;
uniform mat3 modelInverseTranspose;

void main()
{
	Normal = modelInverseTranspose * normal;
	Texcoord = texcoord;
	gl_Position = modelViewProjection * vec4(position, 1.0);
}
