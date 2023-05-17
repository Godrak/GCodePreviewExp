#version 450

layout(location = 1) uniform vec3 cameraPosition;
layout(location = 3) uniform mat4 view;
layout(location = 4) uniform mat4 projection;

layout(location = 0) in int vertex_id; // 0 1 2 3

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

mat2 rotateVec2ToVec2(vec2 fromVector, vec2 toVector) {
    float angle = atan(toVector.y, toVector.x) - atan(fromVector.y, fromVector.x);
    float c = cos(angle);
    float s = sin(angle);
    return mat2(c, -s, s, c);
}

void main() {
    // Retrieve the instance ID
    int id_a = int(gl_InstanceID);
    int id_b = int(gl_InstanceID) + 1;

    vec3 pos_a = loadVec3fromTex(0, id_a);
    vec3 pos_b = loadVec3fromTex(0, id_b);

    vec3 pos_a_vp = (view * vec4(pos_a, 1.0)).xyz;
    vec3 pos_b_vp = (view * vec4(pos_b, 1.0)).xyz;

    vec2 dir_xy_vp = normalize((pos_b_vp - pos_a_vp).xy);

    vec2 additional_space;
    if (vertex_id == 0){
        additional_space = vec2(-1 , -1);
    } else if (vertex_id == 1){
        additional_space = vec2(1 , -1);
    } else if (vertex_id == 2){
        additional_space = vec2(1 , 1);
    } else if (vertex_id == 3){
        additional_space = vec2(-1 , 1);
    }

    mat2 rotation = rotateVec2ToVec2(vec2(0,1), dir_xy_vp);
    additional_space = rotation * additional_space;

    vec3 pos_vp;
    int id;
    if (vertex_id < 2) {
        pos_vp = pos_a_vp;
        id = id_a;
    } else {
        pos_vp = pos_b_vp;
        id = id_b;
    }
    pos_vp += vec3(additional_space, 0.0);

    color = loadVec3fromTex(3, id);

    gl_Position = projection * vec4(pos_vp, 1.0);
}


//   vec3 color_a = loadVec3fromTex(3, id_a);
//     float height_a = texelFetch(pathTexture, ivec2(6, id_a)).x;
//     float width_a = texelFetch(pathTexture, ivec2(7, id_a)).x;

//     vec3 color_b = loadVec3fromTex(3, id_b);
//     float height_b = texelFetch(pathTexture, ivec2(6, id_b)).x;
//     float width_b = texelFetch(pathTexture, ivec2(7, id_b)).x;

// 	vec3 view_dir_a = normalize(pos_a - cameraPosition);
//     vec3 right_vector_a = normalize(cross(view_dir_a, UP));
//     vec3 up_vector_a = normalize(cross(right_vector_a, view_dir_a));

//     vec3 line_dir = pos_b - pos_a;

//     vec3 view_dir_b = normalize(pos_b - cameraPosition);
//     vec3 right_vector_b = normalize(cross(view_dir_b, UP));
//     vec3 up_vector_b = normalize(cross(right_vector_b, view_dir_b));