#version 140

flat in uint id;

out vec4 fragmentColor;

void main() {
    float r = (int(id) % 64) / 64.0;
    float g = ((int(id) + 45) % 128) / 128.0;
    float b = ((int(id) + 356) % 512) / 512.0;

   fragmentColor = vec4(r,g,b,1.0);
}

