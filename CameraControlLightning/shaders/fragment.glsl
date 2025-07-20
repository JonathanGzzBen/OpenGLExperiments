#version 450 core

layout (location = 0) out vec4 fColor;

uniform vec3 objectColor;
uniform vec3 lightPosition;
uniform vec3 viewPos;

in vec3 normal;
in vec3 fragPos;
in vec2 texCoords;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct Light {
    //    vec3 direction;
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;

};

uniform Material material;
uniform Light light;

void main() {
    // Ambient lighting
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, texCoords));

    // Diffuse lighting
    //    vec3 lightDir = normalize(lightPosition - fragPos);
    vec3 lightDir = normalize(light.position - fragPos);
    //    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normalize(normal), lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, texCoords));

    // Specular lighting
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(reflectDir, viewDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * vec3(texture(material.specular, texCoords));


    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    vec3 result = (ambient + diffuse + specular);
    fColor = vec4(result, 1.0);
}