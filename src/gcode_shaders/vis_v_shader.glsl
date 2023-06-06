#version 450

layout(location = 0) uniform mat4 view_projection;

layout(location = 0) in int vertex_id;

// Struct definition for PathPoint
struct PathPoint {
    vec2 pos_xy;
    float pos_z;
    int type;
    float height;
    float width;
};


// Binding point for the path data SSBO
layout(std430, binding = 0) readonly buffer PathBuffer {
    PathPoint points[];
};

// Binding point for the path data SSBO
layout(std430, binding = 1) readonly buffer VisibilityBuffer {
    int visible_ids[];
};

out flat int id_a;

void main() {
    vec3 UP = vec3(0,0,1);

    // Retrieve the instance ID
    id_a = int(gl_InstanceID);
    int id_b = int(gl_InstanceID) + 1;

    // if neither of line endpoints visible, throw away. 
    // (While it may seem confusing, it actually checks visibility of any point of the line, 
    // since we set endpoint A to visible for any fragment that makes to the visiblity framebuffer)
    // if (visible_ids[id_a] == 0 && visible_ids[id_b] == 0) {
    //     gl_Position = vec4(0);
    //     return;
    // }

    vec3 pos_a = vec3(points[id_a].pos_xy, points[id_a].pos_z);
    vec3 pos_b = vec3(points[id_b].pos_xy, points[id_b].pos_z);

    if (points[id_a].width < 0 || points[id_b].width < 0) {
        gl_Position = vec4(0);
        return;
    }

    gl_Position = view_projection * vec4(points[gl_InstanceID + vertex_id].pos_xy, points[gl_InstanceID+vertex_id].pos_z, 1.0);
}
