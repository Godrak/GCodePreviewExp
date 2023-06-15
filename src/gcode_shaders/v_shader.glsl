#version 140

uniform mat4 view_projection;
uniform vec3 camera_position;
uniform int visibility_pass;

uniform samplerBuffer positionsTex;
uniform samplerBuffer heightWidthFlagsTex;
uniform isamplerBuffer segmentIndexTex;

vec3 featureTypeColor(int id) {
	switch (id)
	{
	case  0: return vec3(0.90, 0.70, 0.70); // GCodeExtrusionRole::None
    case  1: return vec3(1.00, 0.90, 0.30); // GCodeExtrusionRole::Perimeter
    case  2: return vec3(1.00, 0.49, 0.22); // GCodeExtrusionRole::ExternalPerimeter
    case  3: return vec3(0.12, 0.12, 1.00); // GCodeExtrusionRole::OverhangPerimeter
    case  4: return vec3(0.69, 0.19, 0.16); // GCodeExtrusionRole::InternalInfill
    case  5: return vec3(0.59, 0.33, 0.80); // GCodeExtrusionRole::SolidInfill
    case  6: return vec3(0.94, 0.25, 0.25); // GCodeExtrusionRole::TopSolidInfill
    case  7: return vec3(1.00, 0.55, 0.41); // GCodeExtrusionRole::Ironing
    case  8: return vec3(0.30, 0.50, 0.73); // GCodeExtrusionRole::BridgeInfill
    case  9: return vec3(1.00, 1.00, 1.00); // GCodeExtrusionRole::GapFill
    case 10: return vec3(0.00, 0.53, 0.43); // GCodeExtrusionRole::Skirt
    case 11: return vec3(0.00, 1.00, 0.00); // GCodeExtrusionRole::SupportMaterial
    case 12: return vec3(0.00, 0.50, 0.00); // GCodeExtrusionRole::SupportMaterialInterface
    case 13: return vec3(0.70, 0.89, 0.67); // GCodeExtrusionRole::WipeTower
    case 14: return vec3(0.37, 0.82, 0.58); // GCodeExtrusionRole::Custom
	default: return vec3(0.00, 0.00, 0.00); // error
	}
}

vec3 travelTypeColor(int id) {
	switch (id)
	{
	case  0: return vec3(0.219, 0.282, 0.609); // Move
	case  1: return vec3(0.112, 0.422, 0.103); // Extrude
	case  2: return vec3(0.505, 0.064, 0.028); // Retract
	default: return vec3(0.000, 0.000, 0.000); // error
	}
}

int extract_role_from_flags(float flags) { return int(round(flags)) & 0xFF; }
int extract_type_from_flags(float flags) { return (int(round(flags)) >> 8) & 0xFF; }

in int vertex_id;

flat out int id_a;
flat out int id_b;
out vec3 pos;
out vec3 color;

void main() {
    vec3 UP = vec3(0,0,1);

    // Retrieve the instance ID
    id_a = int(texelFetch(segmentIndexTex, int(gl_InstanceID)).r);
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

    int id_close = id_a;
    int id_far = id_b;
    float dir_sign = sign(dot(view_b, view_b) - dot(view_a, view_a));
    if (dir_sign < 0) {
        id_close = id_b;
        id_far = id_a;
    }

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
    float h = (visibility_pass == 1) ? 0.4 : 0.5;
    vec3 height_width_flags = texelFetch(heightWidthFlagsTex, id_final).xyz;
    float half_height = h * height_width_flags.x;
    float half_width = h * height_width_flags.y;

    // extend beyond the path points by half width - It allows for seamless connections, 
    // also it better represents the reality (with the caps rounded in frag shader)
    float cap_sign = vertex_id < 4 ? -1.0 : 1.0;
    vec3 cap = half_width * line_dir * dir_sign * cap_sign;

    vec3 horizontal_dir = half_width * right_dir * -sign(dot(view_a, right_dir));
    vec3 vertical_dir = half_height * up_dir * -sign(dot(view_a, up_dir));

    pos = texelFetch(positionsTex, id_final).xyz + cap + hsign * horizontal_dir + vsign * vertical_dir;
    gl_Position = view_projection * vec4(pos, 1.0);
	
	color = (extract_type_from_flags(height_width_flags.z) == 10) ?
		featureTypeColor(extract_role_from_flags(height_width_flags.z)) :
		travelTypeColor(extract_role_from_flags(height_width_flags.z));
}

