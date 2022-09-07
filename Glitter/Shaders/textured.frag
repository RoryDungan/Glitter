#version 150 core

in vec3 FragPos;
in mat3 TBN;
in vec2 Texcoord;

out vec4 outColor;

uniform sampler2D tex;
uniform sampler2D normalMap;

uniform vec3 worldSpaceCameraPos;
uniform vec3 lightPos;

uniform vec3 color = vec3(0.5, 1, 0.5);
uniform vec3 lightColor = vec3(1,1,1);
uniform vec3 ambientColor = vec3(0.1, 0.1, 0.1);
uniform float shininess = 10;
uniform float diffuseMix = 1;
uniform float specularMix = 1;
uniform float normalMapMix = 1;

void main()
{
	vec3 _SpecularColor = vec3(1,1,1);

    vec3 normalMapSample = texture(normalMap, Texcoord).rgb;
    normalMapSample = normalMapSample * 2.0 - 1; // Convert from 0..1 to -1..1
    vec3 normal = normalize(TBN * mix(vec3(0, 0, 1), normalMapSample, normalMapMix)); // transform from tangent to world space

	vec3 lightDir = normalize(lightPos - FragPos);

	// diffuse colour
	vec3 diffuse = lightColor * max(dot(normal, lightDir), 0);

	// specular
	vec3 viewDirection = normalize(worldSpaceCameraPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, normal);
	vec3 specular = _SpecularColor * pow(max(dot(viewDirection, reflectDir), 0), shininess);

	// final light
	vec3 light = ambientColor + diffuse * diffuseMix + specular * specularMix;

	vec3 texSample = texture(tex, Texcoord).rgb;
	// Lets multiply just the color portion (not the alpha)
	// by the light
	outColor = vec4(texSample * light, 1);
}