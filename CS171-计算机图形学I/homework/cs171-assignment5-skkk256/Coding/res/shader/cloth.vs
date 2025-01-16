#version 430 core

// You may need some other layouts.
layout (location = 0) in vec3 Pos;
layout (location = 1) in vec3 Normal;

uniform mat4 Projection;
uniform mat4 View;
// uniform mat4 model;

out VS_OUT {
    vec3 position;
    vec3 normal;
} vs_out;

void main() {
    vs_out.position = Pos;
    vs_out.normal = Normal;

    gl_Position =  Projection * View * vec4(Pos, 1.0);
}