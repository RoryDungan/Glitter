#version 150 core

in vec3 position;
in vec3 normal;
in vec2 texcoord;

out vec3 Normal;
out vec2 Texcoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main()
{
	Normal = mat3(model) * normal;
	Texcoord = texcoord;
	gl_Position = proj * view * model * vec4(position, 1.0);
}
