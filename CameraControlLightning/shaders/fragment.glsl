#version 450 core

layout (location = 0) out vec4 fColor;

uniform vec3 objectColor;
uniform vec3 lightColor;

void main() {
    fColor = vec4(objectColor * lightColor, 1.0);
}