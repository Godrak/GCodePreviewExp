#version 450

#ifdef GL_ES
    #extension GL_EXT_frag_depth : enable
    precision mediump float;
#endif

layout(location = 0) uniform mat4 view_projection;
layout(location = 1) uniform vec3 camera_position;

layout(binding = 1) uniform sampler2DRect pathTexture;

out vec4 fragmentColor;

in flat int id_close;
in flat int id_far;
in vec3 pos;

vec3 loadVec3fromTex(int offset, int pos) {
    vec3 result;
    result.x = texelFetch(pathTexture, ivec2(offset+0, pos)).x;
    result.y = texelFetch(pathTexture, ivec2(offset+1, pos)).x;
    result.z = texelFetch(pathTexture, ivec2(offset+2, pos)).x;

    return result;
} 

#define eta 1e-6
#define PI 3.1415926535897932384626433832795

void nearestPointsOnLineSegments(vec3 a0, vec3 a1, vec3 b0, vec3 b1, out vec3 A, out vec3 B, out float dist) {
    vec3 r = b0 - a0;
    vec3 u = a1 - a0;
    vec3 v = b1 - b0;

    float ru = dot(r, u);
    float rv = dot(r, v);
    float uu = dot(u, u);
    float uv = dot(u, v);
    float vv = dot(v, v);

    float det = uu * vv - uv * uv;
    float s, t;

    if (det < eta * uu * vv) {
        s = clamp(ru / uu, 0.0, 1.0);
        t = 0.0;
    } else {
        s = clamp((ru * vv - rv * uv) / det, 0.0, 1.0);
        t = clamp((ru * uv - rv * uu) / det, 0.0, 1.0);
    }

    float S = clamp((t * uv + ru) / uu, 0.0, 1.0);
    float T = clamp((s * uv - rv) / vv, 0.0, 1.0);

    A = a0 + S * u;
    B = b0 + T * v;
    dist = length(B - A);
}

// Function to get the nearest point on a line segment
vec3 getNearestPointOnLineSegment(vec3 p, vec3 a, vec3 b) {
    vec3 ab = b - a;
    float t = dot(p - a, ab) / dot(ab, ab);
    t = clamp(t, 0.0, 1.0);
    vec3 nearestPoint = a + t * ab;
    return nearestPoint;
}

void main() {
    vec3 UP = vec3(0,0,1);

    vec3 pos_close = loadVec3fromTex(0, id_close);
    vec3 pos_far = loadVec3fromTex(0, id_far);
    float half_height_close = 0.5* texelFetch(pathTexture, ivec2(6, id_close)).x;
    float half_width_close = 0.5*texelFetch(pathTexture, ivec2(7, id_close)).x;
    float half_height_far = 0.5* texelFetch(pathTexture, ivec2(6, id_far)).x;
    float half_width_far = 0.5*texelFetch(pathTexture, ivec2(7, id_far)).x;

    //ray space
    vec3 ray_dir = normalize(pos - camera_position);
    vec3 ray_right_dir = normalize(cross(ray_dir, UP));
    vec3 ray_up_dir = normalize(cross(ray_right_dir, ray_dir));

    // directions of the line box in world space
    vec3 line = pos_far - pos_close;
    float line_len = length(line); 
    vec3 line_dir = line / line_len;
    vec3 line_right_dir = normalize(cross(line_dir, UP));
    vec3 line_up_dir = normalize(cross(line_right_dir, line_dir));

    // find nearest points of ray and line
    vec3 ray_closest;
    vec3 closest;
    float dist;
    float max_distance_inside = 1000*line_len + half_height_far + half_height_close + half_width_close + half_width_far; // conservative limit 
    nearestPointsOnLineSegments(pos_close, pos_far, camera_position, pos + ray_dir * max_distance_inside, closest, ray_closest, dist);
 
    // compute width and height of cross section of the thick line in the plane defined by line and ray
    float wpart = dot(ray_dir, line_right_dir);
    float hpart = dot(ray_dir, line_up_dir);
    vec2 projected = normalize(vec2(abs(wpart), abs(hpart)));
    projected  = vec2(sqrt(projected.x), sqrt(projected.y));

    vec2 wh_close = vec2(projected.x * half_width_close, projected.y * half_height_close);
    vec2 wh_far = vec2(projected.x * half_width_far, projected.y * half_height_far);

    // now also consider the offset from the center, the above width and height work only for rays going through line
    float t = length(closest - pos_close) / line_len;
    float wt = mix(half_width_close, half_width_far, t);
    float ht = mix(half_height_close, half_height_far, t);
    float thickness = length(vec2(projected.y * wt, projected.x * ht));

    float tt = dist / thickness;
    vec3 to_ray = ray_closest - closest;

    float w_sign = -sign(wpart);
    float h_sign = -sign(hpart);

    // Compute new line, which should be at the surface of the thick line and cross the ray
    vec3 thickness_vec_close = wh_close.x * w_sign * line_right_dir + wh_close.y * h_sign * line_up_dir; 
    vec3 thickness_vec_far = wh_far.x * w_sign * line_right_dir + wh_far.y * h_sign * line_up_dir;
    float factor = 1.0;
    vec3 surface_line_close = pos_close + to_ray + thickness_vec_close * factor;
    vec3 surface_line_far = pos_far + to_ray + thickness_vec_far * factor;

    // now compute intersection with the surface line
    vec3 surface_point;
    vec3 ray_point;
    float new_dist;
    nearestPointsOnLineSegments(surface_line_close, surface_line_far, camera_position, camera_position + ray_dir * max_distance_inside, surface_point, ray_point, new_dist);

    vec3 line_nearest = getNearestPointOnLineSegment(surface_point, pos_close, pos_far);
    float t3 = length(line_nearest - pos_close) / line_len;
    float wt3 = mix(half_width_close, half_width_far, t3);
    float ht3 = mix(half_height_close, half_height_far, t3);
    float thickness3 = length(vec2(projected.y * wt3, projected.x * ht3));

    if (dist > thickness3) {discard;}

    vec3 normal = normalize(surface_point - line_nearest);

    vec3 color_close = loadVec3fromTex(3, id_close);
    vec3 color_far = loadVec3fromTex(3, id_far);

    // vec3 color = vec3(new_dist);
    vec3 color = mix(color_close, color_far, length(line_nearest - pos_close) / line_len); 

    vec3 lightColor = vec3(1.0, 1.0, 1.0); // Light color (white)

    vec3 lightDirection = normalize(vec3(0.0, 0.0, 1.0)); // Light direction (assuming (0, 0, 1) for simplicity)

    float diffuseFactor = max(dot(normal, lightDirection), 0.0);
    vec3 diffuse = color * lightColor * diffuseFactor;

    vec4 clip = view_projection * vec4(surface_point, 1.0);

    #ifdef GL_ES
    gl_FragDepthEXT = ((gl_DepthRange.diff * clip.z / clip.w) + gl_DepthRange.near + gl_DepthRange.far) / 2.0;
    #else
    gl_FragDepth = ((gl_DepthRange.diff * clip.z / clip.w) + gl_DepthRange.near + gl_DepthRange.far) / 2.0;
    #endif

    fragmentColor = vec4(diffuse, 1.0);

}
