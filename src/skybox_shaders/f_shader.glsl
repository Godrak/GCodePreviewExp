#version 450

out vec4 fragColor;
in vec3 direction;

void main(){
	vec3 color = vec3(0.2) + 0.6 * normalize(-0.5*direction + vec3(1));
	fragColor = vec4(color, 1.0);
}
