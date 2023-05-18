#version 450

layout(location = 0) uniform mat4 view_projection;
layout(location = 1) uniform vec3 cameraPosition;

layout(location = 0) in int vertex_id;

// Texture binding point for the path data
layout(binding = 1) uniform sampler2DRect pathTexture;

out vec3 color;

vec3 loadVec3fromTex(int offset, int pos) {
    vec3 result;
    result.x = texelFetch(pathTexture, ivec2(offset+0, pos)).x;
    result.y = texelFetch(pathTexture, ivec2(offset+1, pos)).x;
    result.z = texelFetch(pathTexture, ivec2(offset+2, pos)).x;

    return result;
} 

void main() {
    // Retrieve the instance ID
    int id_a = int(gl_InstanceID);
    int id_b = int(gl_InstanceID) + 1;

    vec3 pos_a = loadVec3fromTex(0,id_a);
    vec3 color_a = loadVec3fromTex(3, id_a);
    float height_a = texelFetch(pathTexture, ivec2(6, id_a)).x;
    float width_a = texelFetch(pathTexture, ivec2(7, id_a)).x;

    vec3 pos_b = loadVec3fromTex(0, id_b);
    vec3 color_b = loadVec3fromTex(3, id_b);
    float height_b = texelFetch(pathTexture, ivec2(6, id_b)).x;
    float width_b = texelFetch(pathTexture, ivec2(7, id_b)).x;

	vec3 lineDir = pos_a - pos_b;
    vec3 lineDirNormalized = normalize(lineDir);
    // vec3 upVector = normalize(cross(lineDirNormalized, cameraPosition - pos_a));
    // vec3 rightVector = normalize(cross(lineDirNormalized, upVector));
    vec3 upVector = vec3(0,0,1);
    vec3 rightVector = normalize(cross(lineDirNormalized, upVector));

    // Calculate the four corner points of the box
    // vec3 topLeft = pos_a + upVector * height_a * 0.5 - rightVector * width_a * 0.5;
    // vec3 topRight = pos_a + upVector *  height_a * 0.5 + rightVector * width_a * 0.5;
    // vec3 bottomLeft = pos_b - upVector *  height_b * 0.5 - rightVector * width_b * 0.5;
    // vec3 bottomRight = pos_b - upVector *  height_b * 0.5 + rightVector * width_b * 0.5;

    vec3 topLeft = pos_a + rightVector;
    vec3 topRight = pos_a - rightVector;
    vec3 bottomLeft = pos_b + rightVector;
    vec3 bottomRight = pos_b - rightVector;


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

    gl_Position = view_projection * vec4(vertexPosition, 1.0);
}

