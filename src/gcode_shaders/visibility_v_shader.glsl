#version 140

uniform mat4 view_projection;

uniform usamplerBuffer visible_boxes_bits;

in vec4 pos_and_id;

flat out uint id;

bool get_nth_bit(uint value, int n) {
  uint mask = uint(1) << n;
  return (value & mask) != 0u;
}

void main() {
    int gluint_size = 32;
    id = uint(pos_and_id.w);
    int bits_position = int(id) / gluint_size;
    uint bits = texelFetch(visible_boxes_bits, bits_position).r;
    int bit_position = int(id) % gluint_size; // sizeof(GLuint)
    bool visible = get_nth_bit(bits, bit_position);

    if (visible){
        gl_Position = vec4(0);
    } else {
        gl_Position = view_projection * vec4(pos_and_id.xyz, 1.0);
    }
}

