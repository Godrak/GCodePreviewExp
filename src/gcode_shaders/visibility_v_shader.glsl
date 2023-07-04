#version 140

uniform mat4 view_projection;

uniform isamplerBuffer visible_boxes_heat;

in vec4 pos_and_id;

flat out uint id;

bool get_nth_bit(uint value, int n) {
  uint mask = uint(1) << n;
  return (value & mask) != 0u;
}

void main() {
    id = uint(pos_and_id.w);
    int heat = texelFetch(visible_boxes_heat, int(id)).r;
    if (heat > 4){
        gl_Position = vec4(0);
    } else {
        gl_Position = view_projection * vec4(pos_and_id.xyz, 1.0);
    }
}

