#version 450 core

layout (location = 0) out vec4 fColor;

uniform sampler2D tMomoi;

in vec2 vTexCoord;

void main() {
	// fColor = vec4(1.0, 0.5, 0.5, 1.0);
	fColor = texture(tMomoi, vTexCoord);
}
