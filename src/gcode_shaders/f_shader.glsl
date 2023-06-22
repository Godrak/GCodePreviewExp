#version 140

#ifdef GL_ES
    #extension GL_EXT_frag_depth : enable
    precision mediump float;
#endif

uniform mat4 view_projection;
uniform vec3 camera_position;

uniform samplerBuffer positionsTex;
uniform samplerBuffer heightWidthColorTex;

flat in int id_a;
flat in int id_b;
in vec3 pos;
in vec3 color;

out vec4 fragmentColor;

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
    vec3 pos_a = texelFetch(positionsTex, id_a).xyz;
    vec3 pos_b = texelFetch(positionsTex, id_b).xyz;
    vec3 height_width_color_a = texelFetch(heightWidthColorTex, id_a).xyz;
    vec3 height_width_color_b = texelFetch(heightWidthColorTex, id_b).xyz;

    float half_height_a = 0.5* height_width_color_a.x;
    float half_width_a = 0.5 * height_width_color_a.y;
    float half_height_b = 0.5 * height_width_color_b.x;
    float half_width_b = 0.5 * height_width_color_b.y;

    vec3 radii_a = vec3(half_width_a, half_width_a, half_height_a);
    vec3 radii_b = vec3(half_width_b, half_width_b, half_height_b);

    //ray space
    vec3 ray_dir = normalize(pos - camera_position);
    // directions of the line box in world space
    float line_len = length(pos_a-pos_b); 
    vec3 line = pos_b-pos_a;

    // ################################
    float limit = line_len + max(max(half_height_a, half_width_a), max(half_height_b, half_width_b));
    
    float traversed = 0;
    vec3 surface_point = pos;
    float lt;
    vec3 last_center = getNearestPointOnLineSegment(surface_point, pos_a, line, lt);
    vec3 radii = mix(radii_a, radii_b, lt);
    vec3 s = calculateClosestPointOnEllipsoid(surface_point, last_center, radii);
    float last_depth = length(surface_point - s) * sign(dot(surface_point - s, s - last_center));

    int iter = 0;
    while (traversed < limit && last_depth > 0 && iter < 5) {
        float dist = max(last_depth*0.8, 1e-4); // the 0.8 factor here prevents the ray from going through the line in extreme cases, leaving gaps
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

    vec3 normal = normalize(surface_point - last_center);
    // normal = mix(normal, -ray_dir, clamp(length(camera_position - surface_point)/200.0, 0.0 , 1.0));

    vec3 light_top_dir = vec3(-0.4574957, 0.4574957, 0.7624929);
    float light_top_diffuse = 0.6*0.8;

    vec3 light_front_dir = vec3(-0.4574957, 0.4574957, 0.7624929);
    float light_front_diffuse = 0.6*0.3;

    float ambient = 0.3;

    float diffuseFactor = max(dot(normal, light_top_dir), 0.0);
    float diffuseFactor2 = max(dot(normal, -light_front_dir), 0.0);
    vec3 diffuse = color * (ambient + light_top_diffuse * diffuseFactor + light_front_diffuse * diffuseFactor2);

    vec4 clip = view_projection * vec4(surface_point, 1.0);

    #ifdef GL_ES
    gl_FragDepthEXT = ((gl_DepthRange.diff * clip.z / clip.w) + gl_DepthRange.near + gl_DepthRange.far) / 2.0;
    #else
    gl_FragDepth = ((gl_DepthRange.diff * clip.z / clip.w) + gl_DepthRange.near + gl_DepthRange.far) / 2.0;
    #endif

    fragmentColor = vec4(diffuse, 1.0);

}

