/*
 * gcode.h
 *
 *  Created on: Jun 9, 2019
 *      Author: Pavel Mikus
 *		mail: pavel.mikus@eyen.se
 */

#ifndef GCODE_H_
#define GCODE_H_

#include "globals.h"
#include <cstddef>
#include <future>
#include <glm/fwd.hpp>
#include <vector>
#include <random>
#include <iostream>

namespace gcode {
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
    glm::vec2 pos_xy;
    float pos_z;
    unsigned int flags;
    float height;
    float width;

    unsigned int encode_flags(unsigned int role, unsigned int type) { flags = role << 0 | type << 8; return flags; }
    unsigned int decode_role_from_flags() const { return flags & 0xFF; }
    unsigned int decode_type_from_flags() const { return (flags >> 8) & 0xFF; }
};

enum class VisibilityStatus {READY, RENDERING};

struct BufferedPath
{
    GLuint positions_texture, positions_buffer;
    GLuint height_width_type_texture, height_width_type_buffer;
    GLuint enabled_segments_texture, enabled_segments_buffer;
    size_t enabled_segments_count = 0;
    GLuint visible_segments_texture, visible_segments_buffer;
    size_t visible_segments_count = 0;
    GLuint visibility_pixel_buffer;
    glm::ivec2 visibility_texture_size; 
    
    VisibilityStatus status = VisibilityStatus::READY;
    double time_to_render = 0;
    double time_render_started = 0;
    std::vector<glm::uint32> visiblity_vector{};
    std::future<void> filtering_future{};
};

static unsigned int extract_type_from_flags(unsigned int flags) { return (flags >> 8) & 0xFF; }

BufferedPath bufferExtrusionPaths(const std::vector<PathPoint>& path_points) {
    BufferedPath result;

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> height_width_flags;
    std::vector<glm::uint32> enabled_segments;

    for (size_t i = 0; i < path_points.size(); i++) {
        if (path_points[i].width < 0)
            continue;
    
        if (i + 1 < path_points.size() && path_points[i + 1].width > 0) {
            enabled_segments.push_back(positions.size());
        }
        positions.push_back({path_points[i].pos_xy, path_points[i].pos_z});
        const unsigned int type = path_points[i].decode_type_from_flags();
        const float height = (type == 8) ? 0.1f : path_points[i].height;
        const float width  = (type == 8) ? 0.1f : path_points[i].width;

        height_width_flags.push_back({ height, width, float(path_points[i].flags) });
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
    glGenBuffers(1, &result.height_width_type_buffer);
    glBindBuffer(GL_TEXTURE_BUFFER, result.height_width_type_buffer);

    // buffer data to the path buffer
    glBufferData(GL_TEXTURE_BUFFER, height_width_flags.size() * sizeof(glm::vec3), height_width_flags.data(), GL_STATIC_DRAW);

    // Create and bind the path texture
    glGenTextures(1, &result.height_width_type_texture);
    glBindTexture(GL_TEXTURE_BUFFER, result.height_width_type_texture);

    // Attach the buffer object to the texture buffer
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, result.height_width_type_buffer);

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
    glGenBuffers(1, &result.visible_segments_buffer);
    glGenTextures(1, &result.enabled_segments_texture);
    result.visible_segments_count = 0;
	glGenBuffers(1, &result.visibility_pixel_buffer);

    glBindVertexArray(0);

    return result;
}

BufferedPath generateTestingPathPoints()
{
    std::vector<PathPoint>                pathPoints;
    std::mt19937                          rng(std::random_device{}()); // Random number generator
    std::uniform_real_distribution<float> sizeDist(0.05f, 2.0f);        // Random size distribution
    std::uniform_real_distribution<float> diffDistance(-50.0f, 50.0f);   // Random size distribution

    glm::vec2 last_point = glm::vec2(diffDistance(rng), diffDistance(rng)); 
    for (int i = 0; i < 50; ++i) {
        PathPoint point;
        point.pos_xy = last_point + glm::vec2(diffDistance(rng), diffDistance(rng));
        point.pos_z = 0;
        point.encode_flags(1 + rand() % 12, 10); // role = Random extrusion role, type = Extrude
        point.height = sizeDist(rng);            // Random height between 1.0 and 5.0
        point.width  = 2.0*sizeDist(rng);        // Random width between 1.0 and 5.0
        last_point = point.pos_xy;
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
