#version 450

layout(location = 0) uniform mat4 mvp;
layout(location = 1) uniform vec3 cameraPosition;

layout(location = 0) in int vertex_id;

// Texture binding point for the path data
layout(binding = 0) uniform sampler2DRect pathTexture;

out vec3 color;

vec3 loadVec3fromTex(int pos, int offset) {
    vec3 result;
    result.x = texelFetch(pathTexture, ivec2(pos, offset+0)).x;
    result.y = texelFetch(pathTexture, ivec2(pos, offset+1)).x;
    result.z = texelFetch(pathTexture, ivec2(pos, offset+2)).x;

    return result;
} 

void main2() {
    // Retrieve the instance ID
    int id_a = int(gl_InstanceID);
    int id_b = int(gl_InstanceID) + 1;

    vec3 pos_a = loadVec3fromTex(id_a, 0);
    vec3 color_a = loadVec3fromTex(id_a, 3);
    float height_a = texelFetch(pathTexture, ivec2(id_a, 6)).x;
    float width_a = texelFetch(pathTexture, ivec2(id_a, 7)).x;

    vec3 pos_b = loadVec3fromTex(id_b, 0);
    vec3 color_b = loadVec3fromTex(id_b, 3);
    float height_b = texelFetch(pathTexture, ivec2(id_b, 6)).x;
    float width_b = texelFetch(pathTexture, ivec2(id_b, 7)).x;

	vec3 lineDir = pos_a - pos_b;
    vec3 lineDirNormalized = normalize(lineDir);
    vec3 upVector = normalize(cross(lineDirNormalized, cameraPosition - pos_a));
    vec3 rightVector = normalize(cross(lineDirNormalized, upVector));

    // Calculate the four corner points of the box
    vec3 topLeft = pos_a + upVector * height_a * 0.5 - rightVector * width_a * 0.5;
    vec3 topRight = pos_a + upVector *  height_a * 0.5 + rightVector * width_a * 0.5;
    vec3 bottomLeft = pos_b - upVector *  height_b * 0.5 - rightVector * width_b * 0.5;
    vec3 bottomRight = pos_b - upVector *  height_b * 0.5 + rightVector * width_b * 0.5;

    vec3 vertexPosition = vec3(0,0,0);
    if (vertex_id == 0){
        vertexPosition = topLeft;
        color = color_a;
    }
    else if (vertex_id == 1) {
        vertexPosition = topRight;
        color = color_a;
    }
    else if (vertex_id == 2) {
        vertexPosition = bottomRight;
        color = color_b;
    }
    else if (vertex_id == 3) {
        vertexPosition = bottomLeft;
        color = color_b;
    }

    gl_Position = mvp * vec4(vertexPosition, 1.0);
}


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