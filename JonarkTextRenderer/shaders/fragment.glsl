#version 450 core

layout (location = 0) out vec4 fColor;

in vec2 texPos;

uniform sampler2D font_atlas;

void main() {
    float alpha = texture(font_atlas, texPos).r;
    fColor = vec4(1.0, 1.0, 1.0, alpha);
}