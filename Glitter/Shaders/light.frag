#version 330 core

out vec4 outColor;

uniform vec3 lightColor = vec3(1,1,1);

void main() {
	outColor = vec4(lightColor, 1);
}