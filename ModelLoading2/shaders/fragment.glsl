#version 450 core

layout (location = 0) out vec4 fColor;

in vec3 normal;
in vec3 fragPos;
in vec2 texCoords;

void main() {
    fColor = vec4(1.0);
}