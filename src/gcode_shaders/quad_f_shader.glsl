#version 450

layout (binding = 1, rgba32i) uniform readonly iimage2D visible_instances_texture;

layout(std430, binding = 1) writeonly buffer VisibilityBuffer {
    int visible_ids[];
};

void main() {
     ivec2 texCoords = ivec2(gl_FragCoord.xy);  // Use gl_FragCoord as absolute texture coordinates

    // Read from texture using imageLoad
    int instanceId = imageLoad(visible_instances_texture, texCoords).x;

    // Write the result to the SSBO using imageStore
    visible_ids[instanceId] = 1;

    // Discard the fragment
    discard;
}

