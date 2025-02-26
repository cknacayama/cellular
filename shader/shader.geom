#version 420 core
layout(points) in;
layout(triangle_strip, max_vertices = 36) out;

in VS_OUT {
    vec3 color;
} gs_in[];

out vec3 fragment_color;

uniform mat4 MVP;

const vec4 CUBE_POINTS[8] = {
        vec4(-0.5, -0.5, 0.5, 0.0),
        vec4(0.5, -0.5, 0.5, 0.0),
        vec4(0.5, 0.5, 0.5, 0.0),
        vec4(-0.5, 0.5, 0.5, 0.0),
        vec4(-0.5, -0.5, -0.5, 0.0),
        vec4(0.5, -0.5, -0.5, 0.0),
        vec4(0.5, 0.5, -0.5, 0.0),
        vec4(-0.5, 0.5, -0.5, 0.0)
    };

const int TRIANGLE_INDICES[36] = {
        // Front
        0,
        1,
        2,
        0,
        2,
        3,
        // Back
        5,
        4,
        7,
        5,
        7,
        6,
        // Left
        4,
        0,
        3,
        4,
        3,
        7,
        // Right
        1,
        5,
        6,
        1,
        6,
        2,
        // Top
        3,
        2,
        6,
        3,
        6,
        7,
        // Bottom
        4,
        5,
        1,
        4,
        1,
        0,
    };

void build_cube(vec4 position) {
    fragment_color = gs_in[0].color;
    for (int i = 0; i < 36; i++) {
        gl_Position = MVP * (position + CUBE_POINTS[TRIANGLE_INDICES[i]]);
        EmitVertex();
        if (i + 1 % 3 == 0) {
            EndPrimitive();
        }
    }
}

void main() {
    build_cube(gl_in[0].gl_Position);
}
