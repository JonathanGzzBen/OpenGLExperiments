#version 450 core

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec2 vTexPos;
layout (location = 2) in vec3 vNormal;

uniform mat4 mProjection;
uniform mat4 mView;
uniform mat4 mModel;

out vec3 normal;
out vec3 fragPos;

void main() {
    gl_Position = mProjection * mView * mModel * vec4(vPos, 1.0);
    normal = vNormal;
    normal = transpose(inverse(mat3(mModel))) * vNormal; // Apply normal matrix
    fragPos = vec3(mModel * vec4(vPos, 1.0));
}

