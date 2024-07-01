#version 450 core

layout (location = 0) out vec4 fColor;

in vec2 vTexCoord;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

void main() {
	vec4 diffuseColor = texture(texture_diffuse1, vTexCoord);
	// vec4 specularColor = texture(texture_specular1, vTexCoord);
	// fColor = mix(diffuseColor, specularColor, 0.2);
	fColor = diffuseColor;
}
