#version 450

#ifdef GL_ES
    #extension GL_EXT_frag_depth : enable
    precision mediump float;
#endif

layout(location = 0) uniform mat4 view_projection;
layout(location = 1) uniform vec3 camera_position;

vec3 colors[8];
void initializeColors() {
    colors[0] = vec3(1.0, 0.5, 0.0);
    colors[1] = vec3(1.0, 1.0, 0.0);
    colors[2] = vec3(0.0, 0.0, 1.0);
    colors[3] = vec3(0.0, 1.0, 1.0);
    colors[4] = vec3(1.0, 0.0, 0.0);
    colors[5] = vec3(1.0, 1.0, 1.0);
    colors[6] = vec3(1.0, 0.0, 1.0);
    colors[7] = vec3(0.0, 1.0, 0.0);
}

// Struct definition for PathPoint
struct PathPoint {
    vec2 pos_xy;
    float pos_z;
    int type;
    float height;
    float width;
};

// Binding point for the path data SSBO
layout(std430, binding = 0) buffer PathBuffer {
    PathPoint points[];
};


out vec4 fragmentColor;

in flat int id_a;
in flat int id_b;
in vec3 pos;

// Function to get the nearest point on a line segment
vec3 getNearestPointOnLineSegment(vec3 p, vec3 a, vec3 ab, out float t) {
    t = dot(p - a, ab) / dot(ab, ab);
    t = clamp(t, 0.0, 1.0);
    vec3 nearestPoint = a + t * ab;
    return nearestPoint;
}


vec3 calculateClosestPointOnEllipsoid(vec3 point, vec3 ellipsoidCenter, vec3 ellipsoidRadii) {
    vec3 oc = point - ellipsoidCenter;
    vec3 invRadii = 1.0 / ellipsoidRadii;
    vec3 ocScaled = oc * invRadii;

    // Normalize the scaled vector
    vec3 scaledDirection = normalize(ocScaled);

    // Calculate the closest point on the ellipsoid surface
    vec3 closestPoint = ellipsoidCenter + ellipsoidRadii * scaledDirection;

    return closestPoint;
}

void main() {
    initializeColors();
    // fragmentColor = vec4(colors[points[id_a].type], 1.0); return;
    
    vec3 pos_a = vec3(points[id_a].pos_xy, points[id_a].pos_z);
    vec3 pos_b = vec3(points[id_b].pos_xy, points[id_b].pos_z);

    float half_height_a = 0.5* points[id_a].height;
    float half_width_a = 0.5 * points[id_a].width;
    float half_height_b = 0.5 * points[id_b].height;
    float half_width_b = 0.5 * points[id_b].width;

    vec3 radii_a = vec3(half_width_a, half_width_a, half_height_a);
    vec3 radii_b = vec3(half_width_b, half_width_b, half_height_b);

    //ray space
    vec3 ray_dir = normalize(pos - camera_position);
    // directions of the line box in world space
    float line_len = length(pos_a-pos_b); 
    vec3 line = pos_b-pos_a;
    vec3 line_dir = (pos_a-pos_b) / line_len; 

    vec3 color_a = colors[points[id_a].type];
    vec3 color_b = colors[points[id_b].type];
    // ################################
    float limit = line_len;
    
    float traversed = 0;
    vec3 surface_point = pos;
    float lt;
    vec3 last_center = getNearestPointOnLineSegment(surface_point, pos_a, line, lt);
    vec3 radii = mix(radii_a, radii_b, lt);
    vec3 s = calculateClosestPointOnEllipsoid(surface_point, last_center, radii);
    float last_depth = length(surface_point - s) * sign(dot(surface_point - s, s - last_center));

    int iter = 0;
    while (traversed < limit && last_depth > 0 && iter < 5) {
        float dist = max(last_depth*0.8, 1e-6);
        surface_point = surface_point + ray_dir *  dist;
        traversed += dist;

        vec3 center = getNearestPointOnLineSegment(surface_point, pos_a, line, lt);

        last_center = center;
        radii = mix(radii_a, radii_b, lt);
        s = calculateClosestPointOnEllipsoid(surface_point, center, radii);
        float depth = length(surface_point - s) * sign(dot(surface_point - s, s - last_center));
        if (depth - 1e-4 > last_depth) discard;
        last_depth = depth;
        iter++;
    }

    vec3 color = mix(color_a, color_b, lt);

    vec3 normal = normalize(surface_point - last_center);


    vec3 lightColor = vec3(1.0, 1.0, 1.0); // Light color (white)

    vec3 lightDirection = normalize(vec3(0.0, 0.0, 1.0)); // Light direction (assuming (0, 0, 1) for simplicity)

    float diffuseFactor = max(dot(normal, lightDirection), 0.0);
    float diffuseFactor2 = max(dot(normal, -lightDirection), 0.0);
    vec3 diffuse = color * lightColor * (diffuseFactor + diffuseFactor2);

    vec4 clip = view_projection * vec4(surface_point, 1.0);

    #ifdef GL_ES
    gl_FragDepthEXT = ((gl_DepthRange.diff * clip.z / clip.w) + gl_DepthRange.near + gl_DepthRange.far) / 2.0;
    #else
    gl_FragDepth = ((gl_DepthRange.diff * clip.z / clip.w) + gl_DepthRange.near + gl_DepthRange.far) / 2.0;
    #endif

    fragmentColor = vec4(diffuse, 1.0);

}

