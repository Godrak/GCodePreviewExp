#version 140

uniform mat4 view_projection;
uniform vec3 camera_position;
uniform int visibility_pass;
uniform int instance_base;

uniform samplerBuffer positionsTex;
uniform samplerBuffer heightWidthColorTex;
uniform isamplerBuffer segmentIndexTex;

vec3 decode_color(float color)
{
	int c = int(round(color));
	int r = (c >> 16) & 0xFF;
	int g = (c >> 8) & 0xFF;
	int b = (c >> 0) & 0xFF;
	float f = 1.0 / 255.0f;
    return f * vec3(r, g, b);
}

in int vertex_id;

flat out int id_a;
flat out int id_b;
out vec3 pos;
out vec3 color;

void main() {
    vec3 UP = vec3(0,0,1);

    // Retrieve the instance ID
    int id_position = (visibility_pass == 1) ? instance_base + gl_InstanceID : gl_InstanceID;
    id_a = int(texelFetch(segmentIndexTex, int(id_position)).r);
    id_b = id_a + 1;

    vec3 pos_a = texelFetch(positionsTex, id_a).xyz;
    vec3 pos_b = texelFetch(positionsTex, id_b).xyz;

	vec3 line = pos_b - pos_a;
    vec3 view_a = pos_a - camera_position;
    vec3 view_b = pos_b - camera_position;

    //camera space
    vec3 camera_view_dir = 0.5*(pos_a + pos_b) - camera_position;
    // NOTE: camera dirs are now used only for determining which box size is closer to the camera and should be rendered
    // As such, they do not need normalization for now
    vec3 camera_right_dir_nn = (cross(camera_view_dir, UP));
    vec3 camera_up_dir_nn = (cross(camera_right_dir_nn, camera_view_dir));

    // directions of the line box in world space
    float line_len = length(line);
    vec3 line_dir;
    if (line_len < 1e-4) {
        line_dir = vec3(1.0,0.0,0.0);
    }else {
        line_dir = line / line_len;
    }
    vec3 right_dir;
    if (abs(dot(line_dir, UP)) > 0.9) {
        // For vertical lines, the width and height should be same, there is no concept of up and down.
        // For simplicity, the code will expand width in the x axis, and height in the y axis 
        right_dir = normalize(cross(vec3(1,0,0), line_dir));
    } else {
        right_dir = normalize(cross(line_dir, UP));
    }

    vec3 up_dir = normalize(cross(right_dir, line_dir));

    float dir_sign = sign(dot(view_b, view_b) - dot(view_a, view_a));
    int id_close = (dir_sign < 0) ? id_b : id_a;
    int id_far = (dir_sign < 0) ? id_a : id_b;

    // vertex_position = pos_close + horizontal_dir + vertical_dir;  0 
    // vertex_position = pos_close - horizontal_dir + vertical_dir;  1
    // vertex_position = pos_close - horizontal_dir - vertical_dir;  2 
    // vertex_position = pos_close + horizontal_dir - vertical_dir;  3
    // vertex_position = pos_far + horizontal_dir - vertical_dir;    4
    // vertex_position = pos_far + horizontal_dir + vertical_dir;    5
    // vertex_position = pos_far - horizontal_dir + vertical_dir;    6

    int id_final =  vertex_id < 4 ? id_close : id_far;
    
    float hsign = (vertex_id == 1 || vertex_id == 2 || vertex_id == 6) ? -1.0 : 1.0;
    float vsign = (vertex_id == 2 || vertex_id == 3 || vertex_id == 4) ? -1.0 : 1.0;

    // for visiblity pass, make the lines smaller, so that there are no holes in the result.
    float h = (visibility_pass == 1) ? 0.35 : 0.5;
    vec3 height_width_color = texelFetch(heightWidthColorTex, id_final).xyz;
    float half_height = h * height_width_color.x;
    float half_width = h * height_width_color.y;

    // extend beyond the path points by half width - It allows for seamless connections, 
    // also it better represents the reality (with the caps rounded in frag shader)
    float cap_sign = vertex_id < 4 ? -1.0 : 1.0;
    vec3 cap = half_width * line_dir * dir_sign * cap_sign;

    vec3 horizontal_dir = half_width * right_dir * -sign(dot(view_a, right_dir));
    vec3 vertical_dir = half_height * up_dir * -sign(dot(view_a, up_dir));

	vec3 finalPos = (id_final == id_a) ? pos_a : pos_b;
    pos = finalPos + cap + hsign * horizontal_dir + vsign * vertical_dir;
    gl_Position = view_projection * vec4(pos, 1.0);
	
	color = decode_color(height_width_color.z);
}

