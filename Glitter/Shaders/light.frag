#version 330 core

out vec4 outColor;

uniform vec3 lightColor = vec3(1,1,1);

// Unlit shader with colour based on the light colour. Used to show light position in the scene.
void main() {
	outColor = vec4(lightColor, 1);
}