#version 140

#ifdef GL_ES
    precision mediump float;
#endif

flat in int id_a;
flat in int id_b;
in vec3 pos;
in vec3 color;

out vec4 fragmentColor;

void main() {    
    fragmentColor = vec4(color, 1.0);
}

