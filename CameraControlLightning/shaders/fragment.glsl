#version 450 core

layout (location = 0) out vec4 fColor;

uniform vec3 objectColor;
//uniform vec3 lightColor;
uniform vec3 lightPosition;
uniform vec3 viewPos;

in vec3 normal;
in vec3 fragPos;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

};

uniform Material material;
uniform Light light;

void main() {
    // Ambient lighting
    vec3 ambient = light.ambient * material.ambient;

    // Diffuse lighting
    vec3 lightDir = normalize(lightPosition - fragPos);
    float diff = max(dot(normalize(normal), lightDir), 0.0);
    vec3 diffuse = light.diffuse * (material.diffuse * diff);

    // Specular lighting
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(reflectDir, viewDir), 0.0), material.shininess);
    vec3 specular = light.specular * (material.specular * spec);

    vec3 result = (ambient + diffuse + specular);
    fColor = vec4(result, 1.0);
}