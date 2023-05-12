#version 450

layout(binding = 0) uniform samplerCube skybox_texture;

layout(location = 2) uniform ivec2 screenResolution;

layout(pixel_center_integer) in vec4 gl_FragCoord;

out vec4 fragColor;

in vec3 direction;

void main(){
	vec2 uv = gl_FragCoord.xy/screenResolution;
	float depth = gl_FragCoord.z;
	vec3 skyColor = texture(skybox_texture, direction).xyz;
	
//	if (depth < 0.99999){
//		discard;
//	}

	//color = vec4(depth,depth,depth,1.0);
	fragColor = vec4(skyColor, 1.0);
}
