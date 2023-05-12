#version 450

struct PathPoint {
    vec3 pos;
    vec3 color;
    float height;
    float width;
};

layout(location = 0) uniform mat4 mvp;
layout(location = 1) uniform vec3 cameraPosition;

layout(location = 0) in byte vertex_id;

// Buffer binding point for the SSBO
layout(std430, binding = 0) buffer PathPoints {
    PathPoint pathPoints[];
};

out vec3 fragmentColor; // Output color to fragment shader

void main() {
    // Retrieve the instance ID
    uint instanceID = gl_InstanceID;

    PathPoint a = pathPoints[instanceID];
    PathPoint b = pathPoints[instanceID+1];

	vec3 lineDir = b.pos - a.pos;
    vec3 lineDirNormalized = normalize(lineDir);
    vec3 upVector = normalize(cross(lineDirNormalized, cameraPosition - a.pos));
    vec3 rightVector = normalize(cross(lineDirNormalized, upVector));

    float halfWidthA = a.width * 0.5;
    float halfHeightA = a.height * 0.5;

	float halfWidthB = b.width * 0.5;
    float halfHeightB = b.height * 0.5;

    // Calculate the four corner points of the box
    vec3 topLeft = a.pos + upVector * halfHeightA - rightVector * halfWidthA;
    vec3 topRight = a.pos + upVector * halfHeightA + rightVector * halfWidthA;
    vec3 bottomLeft = lineStart - upVector * halfHeight - rightVector * halfWidth;
    vec3 bottomRight = lineStart - upVector * halfHeight + rightVector * halfWidth;

    // Set the vertex position based on the corner points
    if (gl_VertexID == 0)
        boxVertex = topLeft;
    else if (gl_VertexID == 1)
        boxVertex = topRight;
    else if (gl_VertexID == 2)
        boxVertex = bottomRight;
    else if (gl_VertexID == 3)
        boxVertex = bottomLeft;

    gl_Position = vec4(vertexPosition, 1.0);
}