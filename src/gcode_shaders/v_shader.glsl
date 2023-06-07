#version 450

layout(location = 0) uniform mat4 view_projection;
layout(location = 1) uniform vec3 camera_position;
layout(location = 2) uniform int visibility_pass;

layout(location = 0) in int vertex_id;

layout(binding = 0) uniform samplerBuffer positionsTex;
layout(binding = 1) uniform samplerBuffer heightWidthTypeTex;
layout(binding = 2) uniform samplerBuffer visibilityTex;

out flat int id_a;
out flat int id_b;
out vec3 pos;


float signDotABminusDotAC(vec3 A, vec3 B, vec3 C){
    // sign(dot(A, B) - dot(A, C)) = sign((B.x - C.x) * A.x + (B.y - C.y) * A.y + (B.z - C.z) * A.z)
    return sign((B.x - C.x) * A.x + (B.y - C.y) * A.y + (B.z - C.z) * A.z);
}

void main() {
    vec3 UP = vec3(0,0,1);

    // Retrieve the instance ID
    id_a = int(gl_InstanceID);
    id_b = int(gl_InstanceID) + 1;

    // if not visilibty pass and neither of line endpoints visible, throw away. 
    // (While it may seem confusing, it actually checks visibility of any point of the line, 
    // since we set endpoint A to visible for any fragment that makes to the visiblity framebuffer)
    if (visibility_pass == 0 && texelFetch(visibilityTex, id_a).r == 0 && texelFetch(visibilityTex, id_b).r == 0) {
        gl_Position = vec4(0);
        return;
    }

    // ngeative width means disconnection, drop this line
    if (texelFetch(heightWidthTypeTex, id_a).y < 0 || texelFetch(heightWidthTypeTex, id_b).y < 0) {
        gl_Position = vec4(0);
        return;
    }

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
    vec3 right_dir = normalize(cross(line_dir, UP));
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
    vec2 sizes = texelFetch(heightWidthTypeTex, id_final).xy; 
    float half_height = h * sizes.x;
    float half_width = h * sizes.y;

    // extend beyond the path points by half width - It allows for seamless connections, 
    // also it better represents the reality (with the caps rounded in frag shader)
    float cap_sign = vertex_id < 4 ? -1.0 : 1.0;
    vec3 cap = half_width * line_dir * dir_sign * cap_sign;

    vec3 horizontal_dir = half_width * right_dir * signDotABminusDotAC(camera_right_dir_nn, view_b, view_a);
    vec3 vertical_dir = half_height * up_dir * dir_sign * signDotABminusDotAC(camera_up_dir_nn, view_b, view_a);

    pos = texelFetch(positionsTex, id_final).xyz + cap + hsign * horizontal_dir + vsign * vertical_dir;
    gl_Position = view_projection * vec4(pos, 1.0);
}

