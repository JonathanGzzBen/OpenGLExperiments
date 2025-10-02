#version 450 core

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 texPos;
out vec3 fragPos;
out vec3 normal;

void main() {
    gl_Position = projection * view * model * vec4(vPos, 1.0);
    fragPos = vec3(model * vec4(vPos, 1.0));
    texPos = vTexCoords;
    normal = normalize(transpose(inverse(mat3(model))) * vNormal); // Apply normal matrix
}