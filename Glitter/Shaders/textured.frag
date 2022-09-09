#version 330 core

struct Material {
    sampler2D diffuse;
    sampler2D normal;
    sampler2D specular;
    sampler2D emission;
    float shininess;
};

struct Light {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

in vec3 FragPos;
in mat3 TBN;
in vec2 Texcoord;

out vec4 outColor;

uniform Material material;
uniform Light light;

uniform vec3 worldSpaceCameraPos;


void main()
{
    vec3 normalMapSample = texture(material.normal, Texcoord).rgb;
    normalMapSample = normalMapSample * 2.0 - 1; // Convert from 0..1 to -1..1
    vec3 normal = normalize(TBN * normalMapSample); // transform from tangent to world space
    vec3 toLight = light.position - FragPos;
    vec3 lightDir = normalize(toLight);

    // distance attenuation
    float distanceToLight = length(toLight);
    float attenuation = 1.0 / (
        light.constant + light.linear * distanceToLight + light.quadratic * distanceToLight * distanceToLight
    );

    // spotlight cone
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0, 1);

    // ambient
    vec3 ambient = light.ambient * texture(material.diffuse, Texcoord).rgb;

    // diffuse colour
    float diff = max(dot(normal, lightDir), 0);
    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, Texcoord).rgb;

    // specular
    vec3 viewDirection = normalize(worldSpaceCameraPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDirection, reflectDir), 0), material.shininess);
    vec3 specular =  light.specular * spec * texture(material.specular, Texcoord).rgb;

    // Emission
    vec3 emission = vec3(0);//texture(material.emission, Texcoord).rgb;

    // final light
    vec3 result = ambient * attenuation + 
        diffuse * attenuation * intensity + 
        specular * attenuation * intensity + 
        emission;
    outColor = vec4(result, 1);
}
