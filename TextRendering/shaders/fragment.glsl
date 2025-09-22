#version 450 core

layout (location = 0) out vec4 fColor;

in vec3 normal;
in vec3 fragPos;
in vec2 texCoords;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_diffuse2;
uniform sampler2D texture_diffuse3;
uniform sampler2D texture_specular1;
uniform sampler2D texture_specular2;

void main() {
//    fColor = texture(texture_diffuse1, texCoords);
    fColor = vec4(1.0);
}