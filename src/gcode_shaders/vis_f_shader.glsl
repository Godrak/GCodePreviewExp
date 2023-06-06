#version 450

out int visible_instance_id;

in flat int id_a;

void main() {
   visible_instance_id = id_a;
}

