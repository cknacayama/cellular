#version 420 core
layout(early_fragment_tests) in;

in vec3 fragment_color;
out vec4 frag_colour;

void main() {
    frag_colour = vec4(fragment_color, 1.0);
}
