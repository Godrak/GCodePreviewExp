#version 450

out int visible_instance_id;

in flat int id_a;
in flat int id_b;
in vec3 pos;

void main() {
   visible_instance_id = id_a;
}

