#version 450 core

layout (location = 0) out vec4 fColor;

in vec3 vColor;
in vec2 vTexPos;

uniform sampler2D tex;

void main() {
    if (vColor != vec3(0, 0, 0))
    fColor = vec4(vColor.x, vColor.y, vColor.z, 1.0);
    else
    fColor = texture(tex, vTexPos);
}