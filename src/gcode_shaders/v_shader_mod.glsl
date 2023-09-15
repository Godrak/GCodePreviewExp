#version 150

#define PI      3.1415926538
#define HALF_PI (0.5 * PI)
#define UP      vec3(0.0, 0.0, 1.0)
#define EPSILON 0.001

uniform mat4 view_projection;

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

mat3 rotate_matrix(vec3 axis, float angle)
{
    float cos_theta = cos(angle);
    float sin_theta = sin(angle);
    float one_minus_cos_theta = 1.0 - cos_theta;
    
    float x2 = axis.x * axis.x;
    float y2 = axis.y * axis.y;
    float z2 = axis.z * axis.z;
    float xy = axis.x * axis.y;
    float xz = axis.x * axis.z;
    float yz = axis.y * axis.z;
    
    mat3 rotationMatrix;
    rotationMatrix[0] = vec3(x2 * one_minus_cos_theta + cos_theta, xy * one_minus_cos_theta + axis.z * sin_theta, xz * one_minus_cos_theta - axis.y * sin_theta);
    rotationMatrix[1] = vec3(xy * one_minus_cos_theta - axis.z * sin_theta, y2 * one_minus_cos_theta + cos_theta, yz * one_minus_cos_theta + axis.x * sin_theta);
    rotationMatrix[2] = vec3(xz * one_minus_cos_theta + axis.y * sin_theta, yz * one_minus_cos_theta - axis.x * sin_theta, z2 * one_minus_cos_theta + cos_theta);
    
    return rotationMatrix;
}

vec3 calc_position(vec3 a, vec3 b, vec3 c, float half_width)
{
	vec3 ab_dir = normalize(b - a);
	vec3 bc_dir = normalize(c - b);
	return b + half_width * rotate_matrix(-normalize(cross(ab_dir, bc_dir)), HALF_PI) * normalize(0.5 * (ab_dir + bc_dir));
}

void main() {
    int id_a = texelFetch(segmentIndexTex, gl_InstanceID).r;
    int id_b = id_a + 1;

    vec3 pos_a = texelFetch(positionsTex, id_a).xyz;
    vec3 pos_b = texelFetch(positionsTex, id_b).xyz;
    vec3 hwa_a = texelFetch(heightWidthAngleTex, id_a).xyz;
    vec3 hwa_b = texelFetch(heightWidthAngleTex, id_b).xyz;

    // direction of the line in world space
	vec3 line_dir = normalize(pos_b - pos_a);
	vec3 right_dir;
    if (abs(dot(line_dir, UP)) > 0.9999) {
        // Only travel moves can go in up/down direction.
		// For them, width and height are the same.
		// Any horizontal direction is a valid right direction, we select X direction.
        right_dir = vec3(1.0, 0.0, 0.0);
    }
	else
        right_dir = normalize(cross(line_dir, UP));

    // no need to normalize, the two vectors are already normalized and form a 90 degrees angle
    vec3 up_dir = cross(right_dir, line_dir);

	float half_h_a = 0.5 * hwa_a.x;
	float half_w_a = 0.5 * hwa_a.y;
	float half_h_b = 0.5 * hwa_b.x;
	float half_w_b = 0.5 * hwa_b.y;
	
	vec3 position = vec3(0.0);
    vec3 normal = vec3(2.0); // dummy value
	
	// calculate output position in dependence of vertex id
	if (vertex_id == 0) {
		if (abs(hwa_a.z - PI) < EPSILON) {
			position = pos_a;
			normal = -line_dir;
		}
		else
			position = calc_position(texelFetch(positionsTex, id_a - 1).xyz, pos_a, pos_b, half_w_a);
	}
	else if (vertex_id == 1) { position = pos_a + half_w_a * right_dir; }
	else if (vertex_id == 2) { position = pos_a + half_h_a * up_dir; }
	else if (vertex_id == 3) { position = pos_a - half_w_a * right_dir; }
	else if (vertex_id == 4) { position = pos_a - half_h_a * up_dir; }
	else if (vertex_id == 5) {
		if (abs(hwa_b.z) < EPSILON) {
			position = pos_b;
			normal = line_dir;
		}
		else
			position = calc_position(pos_a, pos_b, texelFetch(positionsTex, id_b + 1).xyz, half_w_b);
	}
	else if (vertex_id == 6) { position = pos_b + half_w_b * right_dir; }
	else if (vertex_id == 7) { position = pos_b + half_h_b * up_dir; }
	else if (vertex_id == 8) { position = pos_b - half_w_b * right_dir; }
	else if (vertex_id == 9) { position = pos_b - half_h_b * up_dir; }

	if (normal.x == 2.0)
		normal = (vertex_id < 5) ? normalize(position - pos_a) : normalize(position - pos_b);

    // LIGHTING begin
    vec3 color_base = decode_color(texelFetch(colorsTex, id_a).x);
	float top_diffuse = light_top_diffuse * max(dot(normal, light_top_dir), 0.0);
    float front_diffuse = light_front_diffuse * max(dot(normal, light_front_dir), 0.0);
    float top_specular = light_top_specular * pow(max(dot(-normalize(position), reflect(-light_top_dir, normal)), 0.0), light_top_shininess);
    color = color_base * (ambient + top_diffuse + front_diffuse + top_specular + emission);
    // LIGHTING end
        
    gl_Position = view_projection * vec4(position, 1.0);
}

