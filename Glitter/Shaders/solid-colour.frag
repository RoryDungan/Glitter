#version 150 core

in mat3 TBN;
in vec2 Texcoord;

out vec4 outColor;

uniform vec3 worldSpaceCameraPos;

uniform vec3 reverseLightDirection;

uniform vec3 color = vec3(0.5, 1, 0.5);
uniform float shininess = 10;
uniform float diffuseMix = 1;
uniform float specularMix = 1;

void main()
{
	vec3 _SunColor = vec3(1,1,1);
	vec3 _SpecularColor = vec3(1,1,1);

	vec3 viewDirection = normalize(worldSpaceCameraPos - gl_FragCoord.xyz);

	vec3 normal = normalize(TBN * vec3(0,0,1));

	// compute the light by taking the dot product
	// of the normal to the light's reverse direction
	vec3 diffuse = _SunColor * dot(normal.xyz, reverseLightDirection);
	vec3 specular = diffuse * _SpecularColor * pow(clamp(dot(reflect(reverseLightDirection, normal), viewDirection), 0, 1), shininess);

	vec3 light = diffuse * diffuseMix + specular * specularMix;

	// Lets multiply just the color portion (not the alpha)
	// by the light
	outColor = vec4(color * light, 1);
}
