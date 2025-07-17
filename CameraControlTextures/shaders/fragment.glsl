#version 450 core

layout (location = 0) out vec4 fColor;

in vec3 vColor;
in vec2 vTexPos;

uniform sampler2D tex0;
uniform sampler2D tex1;

void main() {
    if (vColor != vec3(0, 0, 0))
    fColor = vec4(vColor.x, vColor.y, vColor.z, 1.0);
    else
    fColor = mix(texture(tex0, vTexPos), texture(tex1, vTexPos), 0.8);
//    fColor = texture(tex0, vTexPos);
}