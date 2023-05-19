#version 450

layout(location = 1) uniform vec3 camera_position;

out vec4 fragmentColor;

in vec3 color;
in vec3 pos_a;
in vec3 pos_b;
in vec3 pos;
in float half_height;
in float half_width;
in float camera_dot_up;

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

// Function to get the vertical height of an ellipse at a given x-coordinate
float getEllipseHeight(float x, float w, float h) {
    float xNormalized = x / (w * 0.5); // Normalize x to range [-1, 1]
    float yNormalized = h * sqrt(1.0 - xNormalized * xNormalized); // Calculate y on the ellipse
    return yNormalized;
}

// Function to get the nearest point on a line segment
vec3 getNearestPointOnLineSegment(vec3 p, vec3 a, vec3 b) {
    vec3 ab = b - a;
    float t = dot(p - a, ab) / dot(ab, ab);
    t = clamp(t, 0.0, 1.0);
    vec3 nearestPoint = a + t * ab;
    return nearestPoint;
}

float fixedRightAngleSlerp(float start, float end, float t) {
    return sin((1.0 - t) * PI * 0.5) * start + sin(t * PI * 0.5) * end;
}

void main() {
    vec3 UP = vec3(0,0,1);

    vec3 ray_closest;
    vec3 closest;
    float dist;
    vec3 ray_dir = normalize(pos - camera_position);
    nearestPointsOnLineSegments(pos_a, pos_b, camera_position, camera_position + ray_dir * 1000, closest, ray_closest, dist);
    float dist_limit = fixedRightAngleSlerp(half_width, half_height, abs(camera_dot_up));
    if (dist > dist_limit) {
        discard;
    }

    float h = fixedRightAngleSlerp(half_height, half_width, abs(camera_dot_up));
    float t = (dist / dist_limit);
    float x = h * sqrt(1.0 - t*t);

    float rate_of_entering = 1.0 - abs(dot(ray_dir, normalize(pos_b-pos_a))); 
    x = min(x / rate_of_entering, length(ray_closest - pos));

    vec3 final_pos = ray_closest - x*ray_dir;
    vec3 new_nearest = getNearestPointOnLineSegment(final_pos, pos_a, pos_b);

    vec3 normal = normalize(final_pos - new_nearest);


    vec3 lightColor = vec3(1.0, 1.0, 1.0); // Light color (white)
    vec3 diffuseColor = vec3(0.7, 0.7, 0.7); // Diffuse color (gray)

    vec3 lightDirection = normalize(vec3(0.0, 0.0, 1.0)); // Light direction (assuming (0, 0, 1) for simplicity)

    float diffuseFactor = max(dot(normal, lightDirection), 0.0);
    vec3 diffuse = color * lightColor * diffuseFactor;

    // Set the fragment color to the line color
    fragmentColor = vec4(diffuse, 1.0);
}