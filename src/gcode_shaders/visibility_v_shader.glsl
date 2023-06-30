#version 140

uniform mat4 view_projection;

uniform isamplerBuffer segmentIndexTex;

in vec4 pos_and_id;

flat out uint id;

void main() {
    id = uint(pos_and_id.w);
    gl_Position = view_projection * vec4(pos_and_id.xyz, 1.0);
}

