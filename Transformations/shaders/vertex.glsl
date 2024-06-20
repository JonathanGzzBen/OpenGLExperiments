#version 450 core

layout (location = 0) in vec2 vPos;

uniform mat4 mScale;

void main() {
	gl_Position = mScale * vec4(vPos.x, vPos.y, 0.0, 1.0);
}
