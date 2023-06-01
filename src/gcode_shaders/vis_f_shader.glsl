#version 450


out int instanceID;

in flat int id_a;
in flat int id_b;
in vec3 pos;

void main() {
   instanceID = id_a;
}

