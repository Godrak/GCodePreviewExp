#version 140

flat in int id_a;
flat in int id_b;
in vec3 pos;

out uint visible_instance_id;

void main() {
   visible_instance_id = uint(id_a);
}

