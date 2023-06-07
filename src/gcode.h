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
#include <epoxy/gl_generated.h>
#include <glm/fwd.hpp>
#include <vector>
#include <random>
#include <iostream>

namespace gcode {
GLuint gcodeVAO, vertexBuffer;
GLuint visibilityFrameBuffer, instanceIdsTexture, depthTexture;
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
    int type;
    float height;
    float width;
};

struct BufferedPath
{
    GLuint positions_texture, positions_buffer;
    GLuint height_width_type_texture, height_width_type_buffer;
    GLuint visibility_texture, visibility_buffer;
    size_t point_count;
};

BufferedPath bufferExtrusionPaths(const std::vector<PathPoint>& path_points) {
    BufferedPath result;
    result.point_count = path_points.size();

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> height_width_types;

    for (size_t i = 0; i < path_points.size(); i++) {
        positions.push_back({path_points[i].pos_xy, path_points[i].pos_z});
        height_width_types.push_back({path_points[i].height, path_points[i].width, float(path_points[i].type)});
    }

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
    glBufferData(GL_TEXTURE_BUFFER, height_width_types.size() * sizeof(glm::vec3), height_width_types.data(), GL_STATIC_DRAW);

    // Create and bind the path texture
    glGenTextures(1, &result.height_width_type_texture);
    glBindTexture(GL_TEXTURE_BUFFER, result.height_width_type_texture);

    // Attach the buffer object to the texture buffer
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, result.height_width_type_buffer);

    // Unbind the buffer object and the texture buffer
    glBindBuffer(GL_TEXTURE_BUFFER, 0);
    glBindTexture(GL_TEXTURE_BUFFER, 0);


   // Create a buffer object and bind it to the texture buffer
    glGenBuffers(1, &result.visibility_buffer);
    glBindBuffer(GL_TEXTURE_BUFFER, result.visibility_buffer);

    // buffer data to the path buffer
    glBufferData(GL_TEXTURE_BUFFER, path_points.size() * sizeof(bool), nullptr, GL_DYNAMIC_DRAW);
    bool v = 1;
    glClearBufferData(GL_TEXTURE_BUFFER, GL_R8, GL_RED, GL_UNSIGNED_BYTE, &v);


    // Create and bind the path texture
    glGenTextures(1, &result.visibility_texture);
    glBindTexture(GL_TEXTURE_BUFFER, result.visibility_texture);

    // Attach the buffer object to the texture buffer
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R8, result.visibility_buffer);

    // Unbind the buffer object and the texture buffer
    glBindBuffer(GL_TEXTURE_BUFFER, 0);
    glBindTexture(GL_TEXTURE_BUFFER, 0);


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
        point.type  = rand()%8; // Random color
        point.height = sizeDist(rng);                                             // Random height between 1.0 and 5.0
        point.width  = 2.0*sizeDist(rng);                                             // Random width between 1.0 and 5.0
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
    glDeleteFramebuffers(1, &visibilityFrameBuffer);

    glGenFramebuffers(1, &visibilityFrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, visibilityFrameBuffer);

    glActiveTexture(GL_TEXTURE1);
    glGenTextures(1, &instanceIdsTexture);
    glBindTexture(GL_TEXTURE_2D, instanceIdsTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, globals::visibilityResolution.x, globals::visibilityResolution.y, 0, GL_RED_INTEGER, GL_INT,
                 nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, instanceIdsTexture, 0);

    checkGl();

    glActiveTexture(GL_TEXTURE2);
    glGenTextures(1, &depthTexture);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, globals::visibilityResolution.x, globals::visibilityResolution.y, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glNamedFramebufferTexture(visibilityFrameBuffer, GL_DEPTH_ATTACHMENT, depthTexture, 0);

    checkGl();
    checkGLCall(glCheckNamedFramebufferStatus(visibilityFrameBuffer, GL_FRAMEBUFFER));

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

    glGenFramebuffers(1, &visibilityFrameBuffer);
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
