#version 450 core

layout (location = 0) out vec4 fColor;

in vec3 vColor;

void main() {
    fColor = vec4(vColor.x, vColor.y, vColor.z, 1.0);
}