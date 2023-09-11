#version 150

uniform mat4 view_projection;
uniform vec3 camera_forward;
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

const vec3 light_top_dir = vec3(-0.4574957, 0.4574957, 0.7624929);
const float light_top_diffuse = 0.6*0.8;
const float light_top_specular = 0.6*0.125;
const float light_top_shininess = 20.0;

const vec3 light_front_dir = vec3(0.6985074, 0.1397015, 0.6985074);
const float light_front_diffuse = 0.6*0.3;

const float ambient = 0.3;
const float emission = 0.1;

void main() {
    vec3 UP = vec3(0,0,1);

    int id_a = int(texelFetch(segmentIndexTex, gl_InstanceID).r);
    int id_b = id_a + 1;

    vec3 pos_a = texelFetch(positionsTex, id_a).xyz;
    vec3 pos_b = texelFetch(positionsTex, id_b).xyz;

    // direction of the line in world space
	vec3 line_dir = normalize(pos_b - pos_a);
    vec3 right_dir;
    if (abs(dot(line_dir, UP)) > 0.9) {
        // For vertical lines, the width and height should be same, there is no concept of up and down.
        // For simplicity, the code will expand width in the x axis, and height in the y axis 
        right_dir = normalize(cross(vec3(1,0,0), line_dir));
    } else {
        right_dir = normalize(cross(line_dir, UP));
    }

    vec3 up_dir = normalize(cross(right_dir, line_dir));

    float dir_sign = sign(dot(camera_forward, line_dir));
    int id_close = (dir_sign < 0) ? id_b : id_a;
    int id_far = (dir_sign < 0) ? id_a : id_b;

    // vertex_position = pos_close + horizontal_dir;  0 
    // vertex_position = pos_close + vertical_dir;  1
    // vertex_position = pos_close - horizontal_dir;  2 
    // vertex_position = pos_close - vertical_dir;  3
    // vertex_position = pos_far - vertical_dir;    4
    // vertex_position = pos_far + horizontal_dir;    5
    // vertex_position = pos_far + vertical_dir;    6

    const vec2 horizontal_vertical_view_signs_array[16] = vec2[](
        //horizontal view
        vec2( 1.0,  0.0),
        vec2( 0.0,  1.0),
        vec2(-1.0,  0.0),
        vec2( 0.0, -1.0),
        vec2( 0.0, -1.0),
        vec2( 1.0,  0.0),
        vec2( 0.0,  1.0),
        vec2( 0.0,  1.0),
        // vertical view
        vec2( 0.0,  1.0),
        vec2(-1.0,  0.0),
        vec2( 0.0, -1.0),
        vec2( 1.0,  0.0),
        vec2( 1.0,  0.0),
        vec2( 0.0,  1.0),
        vec2(-1.0,  0.0),
        vec2(-1.0,  0.0)
    );

    int id_final = vertex_id < 4 ? id_close : id_far;

    vec3 close_height_width_angle = texelFetch(heightWidthAngleTex, id_close).xyz;

    vec3 diagonal_dir_border = normalize(close_height_width_angle.x * up_dir + close_height_width_angle.y * right_dir);
    bool is_vertical_view = abs(dot(camera_forward, up_dir)) / abs(dot(diagonal_dir_border, up_dir)) >
        abs(dot(camera_forward, right_dir)) / abs(dot(diagonal_dir_border, right_dir));

    vec2 signs = horizontal_vertical_view_signs_array[vertex_id + 8*int(is_vertical_view)];

    vec3 final_height_width_angle = texelFetch(heightWidthAngleTex, id_final).xyz;
    float half_height = 0.5 * final_height_width_angle.x;
    float half_width = 0.5 * final_height_width_angle.y;

    vec3 horizontal_dir = half_width * right_dir * -sign(dot(camera_forward, right_dir));
    vec3 vertical_dir = half_height * up_dir * -sign(dot(camera_forward, up_dir));

	vec3 segment_pos = (id_final == id_a) ? pos_a : pos_b;
    vec3 pos = segment_pos + signs.x * horizontal_dir + signs.y * vertical_dir;

    if (vertex_id == 2 || vertex_id == 7) {
        float line_dir_sign = id_final == id_a ? -1.0 : 1.0;
        pos = segment_pos + line_dir_sign * line_dir * final_height_width_angle.y * 0.5 * sin(final_height_width_angle.z * 0.5);
        pos += right_dir * final_height_width_angle.y * 0.5 * cos(final_height_width_angle.z * 0.5);
    }

    // LIGHTING begin
    vec3 color_base = decode_color(texelFetch(colorsTex, id_final).x);
    vec3 normal = normalize(pos - segment_pos);

    float top_diffuse = light_top_diffuse * max(dot(normal, light_top_dir), 0.0);
    float front_diffuse = light_front_diffuse * max(dot(normal, light_front_dir), 0.0);
    float top_specular = light_top_specular * pow(max(dot(-normalize(pos), reflect(-light_top_dir, normal)), 0.0), light_top_shininess);
    color = color_base * (ambient + top_diffuse + front_diffuse + top_specular + emission);
    // LIGHTING end
        
    gl_Position = view_projection * vec4(pos, 1.0);
}

