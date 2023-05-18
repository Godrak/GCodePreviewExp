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

float fixedRightAngleSlerp(float start, float end, float t) {
    return sin((1.0 - t) * PI * 0.5) * start + sin(t * PI * 0.5) * end;
}

void main() {
    vec3 UP = vec3(0,0,1);

    vec3 ray_closest;
    vec3 closest;
    float dist;
    vec3 ray_dir = normalize(pos - camera_position);
    nearestPointsOnLineSegments(pos_a, pos_b, camera_position, pos + ray_dir * 1000, closest, ray_closest, dist);
    float dist_limit = fixedRightAngleSlerp(half_width, half_height, camera_dot_up);
    if (dist > dist_limit) {
        discard;
    }

    vec3 n = ray_closest - closest;
    float h = dot(n, UP);






    // Set the fragment color to the line color
    fragmentColor = vec4(color, 1.0);
}