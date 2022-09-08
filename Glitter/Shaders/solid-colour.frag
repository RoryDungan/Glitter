#version 330 core

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
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
in vec2 Texcoord;
in mat3 TBN;

out vec4 outColor;

uniform Material material;
uniform Light light;

uniform vec3 worldSpaceCameraPos;


void main() {
    vec3 normal = normalize(TBN * vec3(0,0,1));
    vec3 toLight = light.position - FragPos;
    vec3 lightDir = normalize(toLight);

    float distanceToLight = length(toLight);
    float attenuation = 1.0 / (
        light.constant + light.linear * distanceToLight + light.quadratic * distanceToLight * distanceToLight
    );

    // spotlight cone
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0, 1);

    // ambient
    vec3 ambient = light.ambient * material.ambient;

    // diffuse colour
    float diff = max(dot(normal, lightDir), 0);
    vec3 diffuse = light.diffuse * (diff * material.diffuse);

    // specular
    vec3 viewDirection = normalize(worldSpaceCameraPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDirection, reflectDir), 0), material.shininess);
    vec3 specular =  light.specular * (spec * material.specular);

    // final light
    vec3 result = ambient * attenuation + 
        diffuse * attenuation * intensity + 
        specular * attenuation * intensity;
    outColor = vec4(result, 1);
}
