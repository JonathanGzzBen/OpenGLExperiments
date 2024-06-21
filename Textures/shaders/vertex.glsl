#version 450 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 vTexCoord;

uniform mat4 mScale;

void main() {
	gl_Position = mScale * vec4(aPos.x, aPos.y, 0.0, 1.0);
	vTexCoord = aTexCoord;
}
