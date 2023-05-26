#version 450

#ifdef GL_ES
    #extension GL_EXT_frag_depth : enable
    precision mediump float;
#endif

layout(location = 0) uniform mat4 view_projection;
layout(location = 1) uniform vec3 camera_position;

layout(binding = 1) uniform sampler2DRect pathTexture;

out vec4 fragmentColor;

in flat int id_a;
in flat int id_b;
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
#define UP vec3(0,0,1)

void nearestPointsOnLineSegments(vec3 a0, vec3 a1, vec3 b0, vec3 b1, out vec3 A, out vec3 B) {
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
}

// Function to get the nearest point on a line segment
vec3 getNearestPointOnLineSegment(vec3 p, vec3 a, vec3 b, out float t) {
    vec3 ab = b - a;
    t = dot(p - a, ab) / dot(ab, ab);
    t = clamp(t, 0.0, 1.0);
    vec3 nearestPoint = a + t * ab;
    return nearestPoint;
}

// Function to calculate the intersection point between a ray and an ellipsoid
vec3 intersectEllipsoid(vec3 origin, vec3 direction, vec3 ellipsoidCenter, vec3 ellipsoidRadii, out float t) {
    vec3 oc = origin - ellipsoidCenter;
    vec3 d = direction;

    // Apply quadratic equation coefficients
    vec3 invRadii = 1.0 / ellipsoidRadii;
    vec3 dScaled = d * invRadii;
    vec3 ocScaled = oc * invRadii;

    float A = dot(dScaled, dScaled);
    float B = 2.0 * dot(dScaled, ocScaled);
    float C = dot(ocScaled, ocScaled) - 1.0;

    float discriminant = B * B - 4.0 * A * C;

    // If the discriminant is negative, there is no intersection
    if (discriminant < 0.0) {
        t = -1;
        float tt = (-B) / (2.0 * A);
        return origin + tt * direction;
    }

    // Calculate the intersection point(s) using the quadratic formula
    float t1 = (-B - sqrt(discriminant)) / (2.0 * A);
    float t2 = (-B + sqrt(discriminant)) / (2.0 * A);

    // Choose the closest intersection point
    t = min(t1, t2);

    return origin + t * direction;
}

vec3 intersectCylinder(vec3 rayOrigin, vec3 rayDirection, vec3 cylinderA, vec3 cylinderB, float cylinderRadius, out float t) {
    vec3 ba = cylinderB - cylinderA;
    vec3 oa = rayOrigin - cylinderA;
    vec3 dir = normalize(rayDirection);
    
    float baba = dot(ba, ba);
    float bard = dot(ba, dir);
    float baoa = dot(ba, oa);
    float rdoa = dot(dir, oa);
    
    float a = baba - bard * bard;
    float b = baba * rdoa - baoa * bard;
    float c = baba * dot(oa, oa) - baoa * baoa - cylinderRadius * cylinderRadius * baba;
    
    float discriminant = b * b - a * c;
    // t = (-b - log(discriminant)) / a;
    // return rayOrigin + t * dir;

    if (discriminant < 0.0) {
        t = (-b) / a;
        return rayOrigin + t * dir;
    } else {
        t = (-b - sqrt(discriminant)) / a;
        return rayOrigin + t * dir;
    }
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

    vec3 pos_a = loadVec3fromTex(0, id_a);
    vec3 pos_b = loadVec3fromTex(0, id_b);
    float half_height_a = 0.5* texelFetch(pathTexture, ivec2(6, id_a)).x;
    float half_width_a = 0.5*texelFetch(pathTexture, ivec2(7, id_a)).x;
    float half_height_b = 0.5* texelFetch(pathTexture, ivec2(6, id_b)).x;
    float half_width_b = 0.5*texelFetch(pathTexture, ivec2(7, id_b)).x;

    //ray space
    vec3 ray_dir = normalize(pos - camera_position);
    vec3 ray_right_dir = normalize(cross(ray_dir, UP));
    vec3 ray_up_dir = normalize(cross(ray_right_dir, ray_dir));

    // directions of the line box in world space
    vec3 line = pos_b - pos_a;
    float line_len = length(line); 
    vec3 line_dir = line / line_len;
    vec3 line_right_dir = normalize(cross(line_dir, UP));
    vec3 line_up_dir = normalize(cross(line_right_dir, line_dir));

    vec3 color_a = loadVec3fromTex(3, id_a);
    vec3 color_b = loadVec3fromTex(3, id_b);

    vec3 radii_a = vec3(half_width_a, half_width_a, half_height_a);
    vec3 radii_b = vec3(half_width_b, half_width_b, half_height_b);

    float lt;
    vec3 center = getNearestPointOnLineSegment(pos, pos_a, pos_b, lt);
    vec3 color = mix(color_a, color_b, lt);
    vec3 radii = mix(radii_a, radii_b, lt);
    vec3 c = calculateClosestPointOnEllipsoid(pos, center, radii);

    float distance_to_center = length(c - center);
    
    float t1;
    vec3 c2 = intersectCylinder(pos, ray_dir, pos_a, pos_b, 0.8*distance_to_center, t1);
    float lt2;
    center = getNearestPointOnLineSegment(c2, pos_a, pos_b, lt2);
    color = mix(color_a, color_b, lt2);
    radii = mix(radii_a, radii_b, lt2);

    float t2;
    vec3 surface_point = intersectEllipsoid(pos - ray_dir, ray_dir, center, radii, t2);
    if (t2<0) discard;
    // vec3 surface_point = c2;

    vec3 cf =surface_point;// mix(surface_point, c, (max(length(c2 - c) - length(radii),0) / line_len));
    vec3 cp = getNearestPointOnLineSegment(cf, pos_a, pos_b, lt2);

    vec3 normal = normalize(cf - cp);


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





// float tl;
    // vec3 center = getNearestPointOnLineSegment(pos, pos_a, pos_b, tl);
    // vec3 radii = mix(radii_a, radii_b, tl);
    // vec3 color = mix(color_a, color_b, tl);
// 
// 
    // float t;
    // vec3 surface_point = intersectEllipsoid(pos - line_len * ray_dir, ray_dir, center, radii, t);
    // vec3 surface_closest = calculateClosestPointOnEllipsoid(surface_point, center, radii);
    // vec3 depth_to_cross = surface_point - surface_closest;
    // 
    // float alfa = acos(dot(line_dir, ray_dir));
    // float dist_to_cross = length(depth_to_cross) * tan(alfa);
// 
    // vec3 c = center + line_dir * dist_to_cross;
    // float tl2 = dot(c - pos_a, line) / dot(line,line);
    // tl2 = clamp(tl2, 0.0, 1.0);
// 
    // center = mix(pos_a,pos_b, tl2);
    // radii = mix(radii_a, radii_b, tl2);
    // color = mix(color_a, color_b, tl2);
// 
    // float t2;
    // surface_point = intersectEllipsoid(pos -line_len * ray_dir, ray_dir, center, radii, t2);
    // if (t2<0) discard;