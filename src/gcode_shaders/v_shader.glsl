#version 150

uniform mat4 view_projection;
uniform vec3 camera_position;
uniform int instance_base;

uniform samplerBuffer positionsTex;
uniform samplerBuffer heightWidthAngleTex;
uniform samplerBuffer colorsTex;
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

out vec3 color;

const vec3  light_top_dir = vec3(-0.4574957, 0.4574957, 0.7624929);
const float light_top_diffuse = 0.6*0.8;
const float light_top_specular = 0.6*0.125;
const float light_top_shininess = 20.0;

const vec3  light_front_dir = vec3(0.6985074, 0.1397015, 0.6985074);
const float light_front_diffuse = 0.6*0.3;

const float ambient = 0.3;
const float emission = 0.1;

const vec3 UP = vec3(0,0,1);

void main() {
    int id_a = texelFetch(segmentIndexTex, gl_InstanceID).r;
    int id_b = id_a + 1;

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

    const vec2 horizontal_vertical_view_signs_array[16] = vec2[](
        //horizontal view (from right)
        vec2( 1.0,  0.0),
        vec2( 0.0,  1.0),
        vec2( 0.0,  0.0),
        vec2( 0.0, -1.0),
        vec2( 0.0, -1.0),
        vec2( 1.0,  0.0),
        vec2( 0.0,  1.0),
        vec2( 0.0,  0.0),
        // vertical view (from top)
        vec2( 0.0,  1.0),
        vec2(-1.0,  0.0),
        vec2( 0.0,  0.0),
        vec2( 1.0,  0.0),
        vec2( 1.0,  0.0),
        vec2( 0.0,  1.0),
        vec2(-1.0,  0.0),
        vec2( 0.0,  0.0)
    );

    int id = vertex_id < 4 ? id_a : id_b;
    vec3 endpoint_pos = vertex_id < 4 ? pos_a : pos_b;
    vec3 height_width_angle = texelFetch(heightWidthAngleTex, id).xyz;

    
    float squared_dist_a = dot(view_a, view_a);
    float squared_dist_b = dot(view_b, view_b);

    vec3 pos_close = (squared_dist_a - squared_dist_b) > 0 ? pos_b : pos_a;
    vec3 camera_view_dir = normalize(pos_close - camera_position);

    vec3 diagonal_dir_border = normalize(height_width_angle.x * up_dir + height_width_angle.y * right_dir);
    bool is_vertical_view = abs(dot(camera_view_dir, up_dir)) / abs(dot(diagonal_dir_border, up_dir)) >
        abs(dot(camera_view_dir, right_dir)) / abs(dot(diagonal_dir_border, right_dir));

    vec2 signs = horizontal_vertical_view_signs_array[vertex_id + 8*int(is_vertical_view)];

    float half_height = 0.5 * height_width_angle.x;
    float half_width = 0.5 * height_width_angle.y;

    vec3 horizontal_dir = half_width * right_dir;
    vec3 vertical_dir = half_height * up_dir;

    vec3 pos = endpoint_pos + signs.x * horizontal_dir + signs.y * vertical_dir;

    if (vertex_id == 2 || vertex_id == 7) {
        float line_dir_sign = (vertex_id == 2) ? -1.0 : 1.0;
        pos += line_dir_sign * line_dir * height_width_angle.y * 0.5 * sin(height_width_angle.z * 0.5);
        pos += right_dir * height_width_angle.y * 0.5 * cos(height_width_angle.z * 0.5);
    }

    // LIGHTING begin
    vec3 color_base = decode_color(texelFetch(colorsTex, id).x);
    vec3 normal = normalize(pos - endpoint_pos);

    float top_diffuse = light_top_diffuse * max(dot(normal, light_top_dir), 0.0);
    float front_diffuse = light_front_diffuse * max(dot(normal, light_front_dir), 0.0);
    float top_specular = light_top_specular * pow(max(dot(-normalize(pos), reflect(-light_top_dir, normal)), 0.0), light_top_shininess);
    color = color_base * (ambient + top_diffuse + front_diffuse + top_specular + emission);
    // LIGHTING end
        
    gl_Position = view_projection * vec4(pos, 1.0);
}

