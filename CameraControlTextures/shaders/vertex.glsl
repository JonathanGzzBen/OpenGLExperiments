#version 450 core

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec2 aTexPos;
layout (location = 2) in vec3 vNormal;

uniform mat4 mProjection;
uniform mat4 mView;
uniform mat4 mModel;

uniform vec3 uColor;

out vec3 vColor;
out vec2 vTexPos;

void main() {
    gl_Position = mProjection * mView * mModel * vec4(vPos, 1.0);
    vColor = uColor;
    vTexPos = aTexPos;
}

