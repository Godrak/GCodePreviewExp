#version 450

layout(location = 0) uniform mat4 view_projection;
layout(location = 1) uniform vec3 camera_position;

layout(location = 0) in int vertex_id;

// Texture binding point for the path data
layout(binding = 1) uniform sampler2DRect pathTexture;

out vec3 color;
out vec3 pos_a;
out vec3 pos_b;
out vec3 pos;
out float half_height;
out float half_width;

vec3 loadVec3fromTex(int offset, int pos) {
    vec3 result;
    result.x = texelFetch(pathTexture, ivec2(offset+0, pos)).x;
    result.y = texelFetch(pathTexture, ivec2(offset+1, pos)).x;
    result.z = texelFetch(pathTexture, ivec2(offset+2, pos)).x;

    return result;
} 

void main() {
    vec3 UP = vec3(0,0,1);

    // Retrieve the instance ID
    int id_a = int(gl_InstanceID);
    int id_b = int(gl_InstanceID) + 1;

    pos_a = loadVec3fromTex(0, id_a);
    pos_b = loadVec3fromTex(0, id_b);

	vec3 line = pos_b - pos_a;
    vec3 view_a = pos_a - camera_position;
    vec3 view_b = pos_b - camera_position;

    //camera space
    vec3 camera_view_dir = 0.5*(pos_a + pos_b) - camera_position;
    vec3 camera_right_dir = normalize(cross(camera_view_dir, UP));
    vec3 camera_up_dir = normalize(cross(camera_right_dir, camera_view_dir));

    // directions of the line box in world space
    vec3 line_dir = normalize(line);
    vec3 right_dir = normalize(cross(line_dir, UP));
    vec3 up_dir = normalize(cross(right_dir, line_dir));

    int id_close = id_a;
    int id_far = id_b;
    vec3 pos_close = pos_a;
    vec3 pos_far = pos_b;
    float dir_sign = sign(dot(view_b, view_b) - dot(view_a, view_a));
    if (dir_sign < 0) {
        id_close = id_b;
        id_far = id_a;
        pos_close = pos_b;
        pos_far = pos_a;
    }

    // vertex_position = pos_close + horizontal_dir + vertical_dir;  0 
    // vertex_position = pos_close - horizontal_dir + vertical_dir;  1
    // vertex_position = pos_close - horizontal_dir - vertical_dir;  2 
    // vertex_position = pos_close + horizontal_dir - vertical_dir;  3
    // vertex_position = pos_far + horizontal_dir - vertical_dir;    4
    // vertex_position = pos_far + horizontal_dir + vertical_dir;    5
    // vertex_position = pos_far - horizontal_dir + vertical_dir;    6

    int final_id = vertex_id < 4 ? id_close : id_far;
    vec3 final_pos = vertex_id < 4 ? pos_close : pos_far;
    
    color = loadVec3fromTex(3, final_id);

    float hsign = (vertex_id == 1 || vertex_id == 2 || vertex_id == 6) ? -1.0 : 1.0;
    float vsign = (vertex_id == 2 || vertex_id == 3 || vertex_id == 4) ? -1.0 : 1.0;

    half_height = 0.5* texelFetch(pathTexture, ivec2(6, final_id)).x;
    half_width = 0.5*texelFetch(pathTexture, ivec2(7, final_id)).x;

    float cap_sign = vertex_id < 4 ? -1.0 : 1.0;
    vec3 cap = (half_width - 0.001) * line_dir * dir_sign * cap_sign;

    vec3 horizontal_dir = half_width * right_dir * sign(dot(view_b, camera_right_dir) - dot(view_a, camera_right_dir));
    vec3 vertical_dir = half_height * up_dir * dir_sign * sign(dot(view_b, camera_up_dir) - dot(view_a, camera_up_dir));

    vec3 vertex_position = final_pos + cap + hsign * horizontal_dir + vsign * vertical_dir;
    pos = vertex_position;
    gl_Position = view_projection * vec4(vertex_position, 1.0);
}

