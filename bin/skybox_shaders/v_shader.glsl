#version 450

layout(location = 0) in vec3 vertex_position;

layout(location = 0) uniform mat4 view_projection;
layout (location = 1) uniform vec3 camera_position;

out vec3 direction;

void main(){
	direction = vertex_position;
	vec3 pos = vertex_position;
	pos = pos + camera_position;	
	
	gl_Position = view_projection*vec4(pos, 1.0);
}
