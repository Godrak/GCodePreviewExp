#version 450

out vec4 fragmentColor;

in vec3 color;

void main() {
    // Set the fragment color to the line color
    fragmentColor = vec4(color, 1.0);
}