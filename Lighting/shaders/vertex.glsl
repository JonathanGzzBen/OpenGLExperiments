#version 450 core

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec2 vTexPos;
layout (location = 2) in vec3 vNormal;

uniform mat4 mView;
uniform mat4 mProjection;

void main() {
    gl_Position = mProjection * mView * vec4(vPos, 1.0);
}

