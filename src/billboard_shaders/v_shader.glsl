#version 450

struct PathPoint {
    vec3 pos;
    vec3 color;
    float height;
    float width;
};

layout(location = 0) uniform mat4 mvp;
layout(location = 1) uniform vec3 cameraPosition;

layout(location = 0) in int vertex_id;

// Buffer binding point for the SSBO
layout(std430, binding = 0) buffer PathPoints {
    PathPoint pathPoints[];
};

out vec3 color;


// void main() {
//     // Retrieve the instance ID
//     uint instanceID = gl_InstanceID;

//     PathPoint a = pathPoints[instanceID];
//     PathPoint b = pathPoints[instanceID+1];

// 	vec3 lineDir = b.pos - a.pos;
//     vec3 lineDirNormalized = normalize(lineDir);
//     vec3 upVector = normalize(cross(lineDirNormalized, cameraPosition - a.pos));
//     vec3 rightVector = normalize(cross(lineDirNormalized, upVector));

//     upVector = vec3(0.0,1.0,0.0);
//     rightVector = normalize(cross(lineDirNormalized, upVector));

//     float halfWidthA = a.width * 0.5;
//     float halfHeightA = a.height * 0.5;

// 	float halfWidthB = b.width * 0.5;
//     float halfHeightB = b.height * 0.5;

//     // Calculate the four corner points of the box
//     vec3 topLeft = a.pos + upVector * halfHeightA - rightVector * halfWidthA;
//     vec3 topRight = a.pos + upVector * halfHeightA + rightVector * halfWidthA;
//     vec3 bottomLeft = b.pos - upVector * halfHeightB - rightVector * halfWidthB;
//     vec3 bottomRight = b.pos - upVector * halfHeightB + rightVector * halfWidthB;

//     vec3 vertexPosition = vec3(0,0,0);
//     if (vertex_id == 0){
//         vertexPosition = topLeft;
//         color = a.color;
//     }
//     else if (vertex_id == 1) {
//         vertexPosition = topRight;
//         color = a.color;
//     }
//     else if (vertex_id == 2) {
//         vertexPosition = bottomRight;
//         color = b.color;
//     }
//     else if (vertex_id == 3) {
//         vertexPosition = bottomLeft;
//         color = b.color;
//     }

//     color = vec3(1,0,0);

//     gl_Position = mvp * vec4(vertexPosition, 1.0);
// }


void main() {
    // Retrieve the instance ID
    uint instanceID = gl_InstanceID;

    vec3 vertexPosition = vec3(0,0,0);
    if (vertex_id == 0){
        vertexPosition = vec3(0,0,0);
        color = vec3(1,0,0);
    }
    else if (vertex_id == 1) {
        vertexPosition = vec3(1,0,0);
        color = vec3(0,1,0);
    }
    else if (vertex_id == 2) {
        vertexPosition = vec3(1,1,0);
        color = vec3(0,0,1);
    }
    else if (vertex_id == 3) {
        vertexPosition = vec3(0,1,0);
        color = vec3(1,1,1);
    }

    gl_Position = vec4(vertexPosition, 1.0);
}