#version 450

layout (location = 0) in vec2 in_position;
layout (location = 1) in vec3 in_color;
layout (location = 2) in float scale;
layout (location = 3) in vec2 offset;

layout (location = 0) out vec3 frag_color;

void main() {
    frag_color = in_color;
    gl_Position = vec4(in_position*scale+offset, 0.0, 1.0);
}
