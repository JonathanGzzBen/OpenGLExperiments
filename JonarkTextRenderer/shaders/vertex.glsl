#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

//uniform mat4 projection;
uniform mat4 projection_view_matrix;

out vec2 texPos;

void main() {
    //    gl_Position = projection_view_matrix * vec4(aPos, 1.0);
    gl_Position = vec4(aPos, 1.0);
    texPos = aTexCoords;
}