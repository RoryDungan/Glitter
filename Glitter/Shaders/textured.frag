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

in VS_OUT {
    vec3 FragPos;
    vec2 Texcoord;
    mat3 TBN;
    vec4 FragPosLightSpace;
} fs_in;

out vec4 outColor;

uniform Material material;
uniform Light light;

uniform sampler2D shadowMap;

uniform vec3 worldSpaceCameraPos;

const float shadowBias = 0.005;

float ShadowCalculation(vec4 fragPosLightSpace) {
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform NDC (-1..1) coords to 0..1
    projCoords = projCoords * 0.5 + 0.5;

    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;

    return currentDepth - shadowBias > closestDepth ? 1.0 : 0.0;
}

void main() {
    vec3 normalMapSample = texture(material.normal, fs_in.Texcoord).rgb;
    normalMapSample = normalMapSample * 2.0 - 1; // Convert from 0..1 to -1..1
    vec3 normal = normalize(fs_in.TBN * normalMapSample); // transform from tangent to world space
    vec3 toLight = light.position - fs_in.FragPos;
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
    vec3 ambient = light.ambient * texture(material.diffuse, fs_in.Texcoord).rgb;

    // diffuse colour
    float diff = max(dot(normal, lightDir), 0);
    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, fs_in.Texcoord).rgb;

    // specular
    vec3 viewDirection = normalize(worldSpaceCameraPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDirection, reflectDir), 0), material.shininess);
    vec3 specular =  light.specular * spec * texture(material.specular, fs_in.Texcoord).rgb;

    // Emission
    vec3 emission = texture(material.emission, fs_in.Texcoord).rgb;

    float shadow = ShadowCalculation(fs_in.FragPosLightSpace);

    // final light
    vec3 result = ambient * attenuation + 
        ((1.0 - shadow) *
            diffuse * attenuation * intensity + 
            specular * attenuation * intensity
        );// + emission;
    outColor = vec4(result, 1);
}
