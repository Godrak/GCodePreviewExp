#version 140

uniform mat4 view_projection;
uniform vec3 camera_position;
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
    int id_position = gl_InstanceID;
    id_a = int(texelFetch(segmentIndexTex, int(id_position)).r);
    id_b = id_a + 1;

    vec3 pos_a = texelFetch(positionsTex, id_a).xyz;
    vec3 pos_b = texelFetch(positionsTex, id_b).xyz;

	vec3 line = pos_b - pos_a;
    vec3 view_a = pos_a - camera_position;
    vec3 view_b = pos_b - camera_position;

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

    // vertex_position = pos_close + horizontal_dir;  0 
    // vertex_position = pos_close + vertical_dir;  1
    // vertex_position = pos_close - horizontal_dir;  2 
    // vertex_position = pos_close - vertical_dir;  3
    // vertex_position = pos_far - vertical_dir;    4
    // vertex_position = pos_far + horizontal_dir;    5
    // vertex_position = pos_far + vertical_dir;    6

    // third coordinate is -1 for vertices that should use id_close pos, and 1 for id_far vertices
    const vec3 horizontal_vertical_view_signs_array[14] = vec3[](
        vec3( 1.0,  0.0, -1.0),
        vec3( 0.0,  1.0, -1.0),
        vec3(-1.0,  0.0, -1.0),
        vec3( 0.0, -1.0, -1.0),
        vec3( 0.0, -1.0,  1.0),
        vec3( 1.0,  0.0,  1.0),
        vec3( 0.0,  1.0,  1.0),
        // vertical view signs and id
        vec3( 0.0,  1.0, -1.0),
        vec3( 0.0,  1.0,  1.0),
        vec3(-1.0,  0.0,  1.0),
        vec3(-1.0,  0.0, -1.0),
        vec3( 0.0, -1.0, -1.0),
        vec3( 1.0,  0.0, -1.0),
        vec3( 1.0,  0.0,  1.0)
    );

    vec3 camera_view_dir = 0.5*(pos_a + pos_b) - camera_position;
    bool is_horizontal_view = abs(dot(camera_view_dir, right_dir)) > abs(dot(camera_view_dir, up_dir));

    vec3 signs = horizontal_vertical_view_signs_array[vertex_id + 7*int(!is_horizontal_view)];

    int id_final = signs.z < 0 ? id_close : id_far;
    
    vec3 height_width_color = texelFetch(heightWidthColorTex, id_final).xyz;
    float half_height = 0.5 * height_width_color.x;
    float half_width = 0.5 * height_width_color.y;

    // extend beyond the path points by half width - It allows for seamless connections, 
    // also it better represents the reality (with the caps rounded in frag shader)
    float cap_sign = signs.z;
    vec3 cap = 0.5 * half_width * line_dir * dir_sign * cap_sign;

    vec3 horizontal_dir = half_width * right_dir * -sign(dot(view_a, right_dir));
    vec3 vertical_dir = half_height * up_dir * -sign(dot(view_a, up_dir));

	vec3 segment_pos = (id_final == id_a) ? pos_a : pos_b;
    pos = segment_pos + signs.x * horizontal_dir + signs.y * vertical_dir;
    
    //LIGHT
	vec3 color_base = decode_color(height_width_color.z);
    vec3 normal = normalize(pos - segment_pos);

    vec3 light_top_dir = vec3(-0.4574957, 0.4574957, 0.7624929);
    float light_top_diffuse = 0.6*0.8;

    vec3 light_front_dir = vec3(-0.4574957, 0.4574957, 0.7624929);
    float light_front_diffuse = 0.6*0.3;

    float ambient = 0.3;

    float diffuseFactor = max(dot(normal, light_top_dir), 0.0);
    float diffuseFactor2 = max(dot(normal, -light_front_dir), 0.0);
    color = color_base * (ambient + light_top_diffuse * diffuseFactor + light_front_diffuse * diffuseFactor2);
    // LIGHT end
        
    if (vertex_id == 1 || vertex_id == 3) pos += cap;
    gl_Position = view_projection * vec4(pos, 1.0);
}

