#version 150 core

in mat3 TBN;
in vec2 Texcoord;

out vec4 outColor;

uniform sampler2D tex;
uniform sampler2D normalMap;

uniform vec3 reverseLightDirection;

void main()
{
	// because v_normal is a varying it's interpolated
	// so it will not be a uint vector. Normalizing it
	// will make it a unit vector again
	//vec3 normal = normalize(Normal);

    vec3 normal = texture(normalMap, Texcoord).rgb;
    normal = normalize(normal * 2.0 - 1); // Convert from 0..1 to -1..1
    normal = normalize(TBN * normal); // transform from tangent to world space

	// compute the light by taking the dot product
	// of the normal to the light's reverse direction
	float light = dot(normal.rgb, reverseLightDirection);

	// Lets multiply just the color portion (not the alpha)
	// by the light
	outColor = texture(tex, Texcoord);
	outColor.rgb *= light;

    outColor.rgb = (normal + 1) / 2;
}
