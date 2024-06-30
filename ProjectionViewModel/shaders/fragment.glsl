#version 450 core

layout (location = 0) out vec4 fColor;

uniform sampler2D tMomoi;

in vec2 vTexCoord;

void main() {
	fColor = texture(tMomoi, vTexCoord);
}
