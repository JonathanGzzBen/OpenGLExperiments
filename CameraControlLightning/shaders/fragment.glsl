#version 450 core

layout (location = 0) out vec4 fColor;

uniform vec3 objectColor;
uniform vec3 lightColor;

void main() {
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    vec3 result = objectColor * ambient;
    fColor = vec4(result, 1.0);
}