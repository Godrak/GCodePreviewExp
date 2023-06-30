#version 140

flat in uint id;

out uint fragment_id;

void main() {
    fragment_id = id;
}

