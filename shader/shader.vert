#version 420 core
in vec3 vertex_position;
in vec3 vertex_color;

out VS_OUT {
    vec3 color;
} vs_out;

void main() {
    vs_out.color = vertex_color;
    gl_Position = vec4(vertex_position, 1.0);
}
