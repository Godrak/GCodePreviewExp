/*
 * gcode.h
 *
 *  Created on: Jun 9, 2019
 *      Author: Pavel Mikus
 *		mail: pavel.mikus@eyen.se
 */

#ifndef GCODE_H_
#define GCODE_H_

#include "glad/glad.h"
#include "globals.h"
#include <cstddef>
#include <future>
#include <glm/fwd.hpp>
#include <vector>
#include <random>
#include <iostream>

namespace gcode {

static unsigned int extract_role_from_flags(unsigned int flags) { return flags & 0xFF; }
static unsigned int extract_type_from_flags(unsigned int flags) { return (flags >> 8) & 0xFF; }

GLuint gcodeVAO, vertexBuffer;
GLuint visibilityFramebuffer, instanceIdsTexture, depthTexture;
GLuint quadVAO;

GLuint pathSSBObindPoint = 5;

GLint vid_loc = 0;

size_t vertex_data_size = 18;
int vertex_data[] = {  
        0, 1, 2,  // left side of box (for common depiction of a box, where left, right and top sides are visible)
        0, 2, 3,
        0, 3, 4,  // right side of box    
        0, 4, 5, 
        0, 5, 6, // top side of box 
        0, 6, 1 };

struct PathPoint
{
    glm::vec3 position;
    unsigned int flags;
    float height;
    float width;

    unsigned int encode_flags(unsigned int role, unsigned int type) { flags = role << 0 | type << 8; return flags; }
    unsigned int role_from_flags() const { return extract_role_from_flags(flags); }
    unsigned int type_from_flags() const { return extract_type_from_flags(flags); }
};

enum class VisibilityStatus {READY, RENDERING};

struct BufferedPath
{
    GLuint positions_texture, positions_buffer;
    GLuint height_width_color_texture, height_width_color_buffer;
    GLuint enabled_segments_texture, enabled_segments_buffer;
    size_t enabled_segments_count = 0;
    GLuint visible_segments_texture;
    std::pair<GLuint, GLuint> visible_segments_doublebuffer;
    std::pair<size_t, size_t> visible_segments_counts = {0,0};
    GLuint visibility_pixel_buffer;
    glm::ivec2 visibility_texture_size; 
    
    std::vector<glm::uint32> visiblity_vector{};
    std::future<void> filtering_future{};
    GLsync rendering_sync_fence;
    GLsync buffering_sync_fence;
};

const std::vector<std::array<float, 4>> Extrusion_Role_Colors{ {
    { 0.90f, 0.70f, 0.70f, 1.0f },   // None
    { 1.00f, 0.90f, 0.30f, 1.0f },   // Perimeter
    { 1.00f, 0.49f, 0.22f, 1.0f },   // ExternalPerimeter
    { 0.12f, 0.12f, 1.00f, 1.0f },   // OverhangPerimeter
    { 0.69f, 0.19f, 0.16f, 1.0f },   // InternalInfill
    { 0.59f, 0.33f, 0.80f, 1.0f },   // SolidInfill
    { 0.94f, 0.25f, 0.25f, 1.0f },   // TopSolidInfill
    { 1.00f, 0.55f, 0.41f, 1.0f },   // Ironing
    { 0.30f, 0.50f, 0.73f, 1.0f },   // BridgeInfill
    { 1.00f, 1.00f, 1.00f, 1.0f },   // GapFill
    { 0.00f, 0.53f, 0.43f, 1.0f },   // Skirt
    { 0.00f, 1.00f, 0.00f, 1.0f },   // SupportMaterial
    { 0.00f, 0.50f, 0.00f, 1.0f },   // SupportMaterialInterface
    { 0.70f, 0.89f, 0.67f, 1.0f },   // WipeTower
    { 0.37f, 0.82f, 0.58f, 1.0f },   // Custom
} };

const std::vector<std::array<float, 4>> Travel_Colors{ {
    { 0.219f, 0.282f, 0.609f, 1.0f }, // Move
    { 0.112f, 0.422f, 0.103f, 1.0f }, // Extrude
    { 0.505f, 0.064f, 0.028f, 1.0f }  // Retract
} };

void updatePathColors(const BufferedPath& path, const std::vector<PathPoint>& path_points)
{
    auto select_color = [](unsigned int flags) {
        static const std::array<float, 4> error_color = { 0.5f, 0.5f, 0.5f, 1.0f };
        const unsigned int role = extract_role_from_flags(flags);
        const unsigned int type = extract_type_from_flags(flags);
        switch (config::visualization_type)
        {
        case 0: // Feature type
        {
            switch (type)
            {
            case 8: // Travel
            {
                assert(role < Travel_Colors.size());
                return Travel_Colors[role];
            }
            case 10: // Extrude
            {
                assert(role < Extrusion_Role_Colors.size());
                return Extrusion_Role_Colors[role];
            }
            }
            break;
        }
        }

        return error_color;
    };

    auto format_color = [&](const PathPoint& point) {
        const std::array<float, 4> color = select_color(point.flags);
        const int r = (int)(255.0f * color[0]);
        const int g = (int)(255.0f * color[1]);
        const int b = (int)(255.0f * color[2]);
        const int a = (int)(255.0f * color[3]);
        const int i_color = r << 24 | g << 16 | b << 8 | a;
        return float(i_color);
    };

    assert(path.height_width_color_buffer > 0);
    glBindBuffer(GL_TEXTURE_BUFFER, path.height_width_color_buffer);
    const char* ptr = (const char*)glMapBuffer(GL_TEXTURE_BUFFER, GL_WRITE_ONLY);

    assert(ptr != nullptr);

    for (size_t i = 0; i < path_points.size(); i++) {
        const float color = format_color(path_points[i]);
        const size_t offset = (i * 3 + 2) * sizeof(float);
        memcpy((void*)(ptr + offset), (const void*)&color, sizeof(color));
    }

    glBindBuffer(GL_TEXTURE_BUFFER, 0);
}

BufferedPath bufferExtrusionPaths(const std::vector<PathPoint>& path_points) {
    BufferedPath result;

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> height_width_color;
    std::vector<glm::uint32> enabled_segments;

    for (size_t i = 0; i < path_points.size(); i++) {
        if (i + 1 < path_points.size() && path_points[i + 1].position != path_points[i].position) {
            enabled_segments.push_back((glm::uint32)positions.size());
        }
        positions.push_back({path_points[i].position});
        const unsigned int type = path_points[i].type_from_flags();
        const float height = (type == 8) ? 0.1f : path_points[i].height;
        const float width  = (type == 8) ? 0.1f : path_points[i].width;

        height_width_color.push_back({ height, width, 0.0f }); // color will be set later with updatePathColors()
    }

    result.enabled_segments_count = enabled_segments.size();

    glBindVertexArray(gcodeVAO);

    // Create a buffer object and bind it to the texture buffer
    glGenBuffers(1, &result.positions_buffer);
    glBindBuffer(GL_TEXTURE_BUFFER, result.positions_buffer);

    // buffer data to the path buffer
    glBufferData(GL_TEXTURE_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_STATIC_DRAW);

    // Create and bind the path texture
    glGenTextures(1, &result.positions_texture);
    glBindTexture(GL_TEXTURE_BUFFER, result.positions_texture);

    // Attach the buffer object to the texture buffer
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, result.positions_buffer);

    // Unbind the buffer object and the texture buffer
    glBindBuffer(GL_TEXTURE_BUFFER, 0);
    glBindTexture(GL_TEXTURE_BUFFER, 0);

     // Create a buffer object and bind it to the texture buffer
    glGenBuffers(1, &result.height_width_color_buffer);
    glBindBuffer(GL_TEXTURE_BUFFER, result.height_width_color_buffer);

    // buffer data to the path buffer
    glBufferData(GL_TEXTURE_BUFFER, height_width_color.size() * sizeof(glm::vec3), height_width_color.data(), GL_DYNAMIC_DRAW);

    // Create and bind the path texture
    glGenTextures(1, &result.height_width_color_texture);
    glBindTexture(GL_TEXTURE_BUFFER, result.height_width_color_texture);

    // Attach the buffer object to the texture buffer
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, result.height_width_color_buffer);

    // Unbind the buffer object and the texture buffer
    glBindBuffer(GL_TEXTURE_BUFFER, 0);
    glBindTexture(GL_TEXTURE_BUFFER, 0);

   // Create a buffer object and bind it to the texture buffer
    glGenBuffers(1, &result.enabled_segments_buffer);
    glBindBuffer(GL_TEXTURE_BUFFER, result.enabled_segments_buffer);

    // buffer data to the path buffer
    glBufferData(GL_TEXTURE_BUFFER, enabled_segments.size() * sizeof(glm::uint32), enabled_segments.data(), GL_STATIC_DRAW);

    // Create and bind the path texture
    glGenTextures(1, &result.enabled_segments_texture);
    glBindTexture(GL_TEXTURE_BUFFER, result.enabled_segments_texture);

    // Attach the buffer object to the texture buffer
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R32UI, result.enabled_segments_buffer);

    // Unbind the buffer object and the texture buffer
    glBindBuffer(GL_TEXTURE_BUFFER, 0);
    glBindTexture(GL_TEXTURE_BUFFER, 0);

    // Create a buffer object and bind it to the texture buffer
    glGenBuffers(1, &result.visible_segments_doublebuffer.first);
    glGenBuffers(1, &result.visible_segments_doublebuffer.second);
    glGenTextures(1, &result.enabled_segments_texture);
    glGenBuffers(1, &result.visibility_pixel_buffer);

    result.rendering_sync_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    result.buffering_sync_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

    glBindVertexArray(0);

    updatePathColors(result, path_points);

    return result;
}

BufferedPath generateTestingPathPoints()
{
    std::vector<PathPoint>                pathPoints;
    std::mt19937                          rng(std::random_device{}()); // Random number generator
    std::uniform_real_distribution<float> sizeDist(0.05f, 2.0f);        // Random size distribution
    std::uniform_real_distribution<float> diffDistance(-50.0f, 50.0f);   // Random size distribution

    glm::vec3 last_point = glm::vec3(diffDistance(rng), diffDistance(rng), 0); 
    for (int i = 0; i < 50; ++i) {
        PathPoint point;
        point.position = last_point + glm::vec3(diffDistance(rng), diffDistance(rng), diffDistance(rng));
        point.encode_flags(1 + rand() % 12, 10); // role = Random extrusion role, type = Extrude
        point.height = sizeDist(rng);            // Random height between 1.0 and 5.0
        point.width  = point.height;//2.0*sizeDist(rng);        // Random width between 1.0 and 5.0
        last_point = point.position;
        pathPoints.push_back(point);
    }

   return bufferExtrusionPaths(pathPoints);
}

void recreateVisibilityBufferOnResolutionChange()
{
    // Delete existing textures and framebuffer
    glDeleteTextures(1, &instanceIdsTexture);
    glDeleteTextures(1, &depthTexture);
    glDeleteFramebuffers(1, &visibilityFramebuffer);

    glGenFramebuffers(1, &visibilityFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, visibilityFramebuffer);

    glActiveTexture(GL_TEXTURE1);
    glGenTextures(1, &instanceIdsTexture);
    glBindTexture(GL_TEXTURE_2D, instanceIdsTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, globals::visibilityResolution.x, globals::visibilityResolution.y, 0, GL_RED_INTEGER, GL_UNSIGNED_INT,
                 nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, instanceIdsTexture, 0);

    checkGl();

    glActiveTexture(GL_TEXTURE2);
    glGenTextures(1, &depthTexture);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, globals::visibilityResolution.x, globals::visibilityResolution.y, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

    checkGl();
    checkGLCall(glCheckFramebufferStatus(visibilityFramebuffer));

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void init()
{
    glGenVertexArrays(1, &gcodeVAO);
    glBindVertexArray(gcodeVAO);

    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

    checkGl();

    glBufferData(GL_ARRAY_BUFFER, vertex_data_size * sizeof(int), vertex_data, GL_STATIC_DRAW);

    // vertex attributes - id:
    glEnableVertexAttribArray(vid_loc);
    glVertexAttribIPointer(vid_loc, 1, GL_INT, sizeof(int), (void *)0);
    checkGl();

    glBindVertexArray(0);

    glGenFramebuffers(1, &visibilityFramebuffer);
    glGenTextures(1, &instanceIdsTexture);
  	glGenTextures(1, &depthTexture);
    recreateVisibilityBufferOnResolutionChange();

    glGenVertexArrays(1, &quadVAO);
    glBindVertexArray(quadVAO);

    checkGl();

    glBindVertexArray(0);
}


} // namespace gcode

#endif /* GCODE_H_ */
