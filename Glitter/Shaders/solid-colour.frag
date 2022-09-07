#version 330 core

in vec3 FragPos;
in vec2 Texcoord;
in mat3 TBN;

out vec4 outColor;

uniform vec3 worldSpaceCameraPos;
uniform vec3 lightPos;

//uniform vec3 reverseLightDirection;

uniform vec3 color = vec3(0.5, 1, 0.5);
uniform vec3 lightColor = vec3(1,1,1);
uniform vec3 ambientColor = vec3(0.1, 0.1, 0.1);
uniform float shininess = 10;
uniform float diffuseMix = 1;
uniform float specularMix = 1;

void main()
{
	vec3 _SpecularColor = vec3(1,1,1);

	vec3 normal = normalize(TBN * vec3(0,0,1));

	vec3 lightDir = normalize(lightPos - FragPos);

	// diffuse colour
	vec3 diffuse = lightColor * max(dot(normal, lightDir), 0);

	// specular
	vec3 viewDirection = normalize(worldSpaceCameraPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, normal);
	vec3 specular = _SpecularColor * pow(max(dot(viewDirection, reflectDir), 0), shininess);

	vec3 light = ambientColor + diffuse * diffuseMix + specular * specularMix;

	// Lets multiply just the color portion (not the alpha)
	// by the light
	outColor = vec4(color * light, 1);
}
