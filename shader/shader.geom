#version 420 core
layout(points) in;
layout(triangle_strip, max_vertices = 14) out;

in VS_OUT {
    vec3 color;
} gs_in[];

out vec3 fragment_color;

uniform mat4 MVP;

const vec4 CUBE_STRIP[14] = {
    vec4(+0.5, +0.5, -0.5, 0.0), // Back-top-right
    vec4(-0.5, +0.5, -0.5, 0.0), // Back-top-left
    vec4(+0.5, -0.5, -0.5, 0.0), // Back-bottom-right
    vec4(-0.5, -0.5, -0.5, 0.0), // Back-bottom-left
    vec4(-0.5, -0.5, +0.5, 0.0), // Front-bottom-left
    vec4(-0.5, +0.5, -0.5, 0.0), // Back-top-left
    vec4(-0.5, +0.5, +0.5, 0.0), // Front-top-left
    vec4(+0.5, +0.5, -0.5, 0.0), // Back-top-right
    vec4(+0.5, +0.5, +0.5, 0.0), // Front-top-right
    vec4(+0.5, -0.5, -0.5, 0.0), // Back-bottom-right
    vec4(+0.5, -0.5, +0.5, 0.0), // Front-bottom-right
    vec4(-0.5, -0.5, +0.5, 0.0), // Front-bottom-left
    vec4(+0.5, +0.5, +0.5, 0.0), // Front-top-right
    vec4(-0.5, +0.5, +0.5, 0.0) // Front-top-left
};

void main() {
    vec4 position = gl_in[0].gl_Position;
    fragment_color = gs_in[0].color;
    for (uint i = 0; i < 14; i++) {
        gl_Position = MVP * (position + CUBE_STRIP[i]);
        EmitVertex();
    }
    EndPrimitive();
}
