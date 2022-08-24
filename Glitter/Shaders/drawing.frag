#version 150 core

in vec3 Normal;
in vec2 Texcoord;

out vec4 outColor;

uniform sampler2D tex;

uniform vec3 reverseLightDirection;

void main()
{
	// because v_normal is a varying it's interpolated
	// so it will not be a uint vector. Normalizing it
	// will make it a unit vector again
	vec3 normal = normalize(Normal);

	// compute the light by taking the dot product
	// of the normal to the light's reverse direction
	float light = dot(normal, reverseLightDirection);

	// Lets multiply just the color portion (not the alpha)
	// by the light
	outColor = texture(tex, Texcoord);// * vec4(Color, 1.0);
	outColor.rgb *= light;
}
