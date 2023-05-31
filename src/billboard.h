/*
 * billboard.h
 *
 *  Created on: Jun 9, 2019
 *      Author: Pavel Mikus
 *		mail: pavel.mikus@eyen.se
 */

#ifndef BILLBOARD_H_
#define BILLBOARD_H_

#include <epoxy/gl_generated.h>
#include "glm/glm.hpp"
#include <glm/fwd.hpp>
#include <vector>
#include <random>
#include <iostream>
#include "globals.h"

namespace billboard {
GLuint billboardVAO, vertexBuffer;

GLuint pathSSBObindPoint = 5;

GLint vid_loc = 0;

std::vector<int> vertexData;

void prepareData()
{
    vertexData = std::vector<int>{
        0, 1, 2,  // left side of box (for common depiction of a box, where left, right and top sides are visible)
        0, 2, 3,
        0, 3, 4,  // right side of box    
        0, 4, 5, 
        0, 5, 6, // top side of box 
        0, 6, 1 
        };
}

//https://stackoverflow.com/questions/38172696/should-i-ever-use-a-vec3-inside-of-a-uniform-buffer-or-shader-storage-buffer-o
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
    GLuint path_buffer;
    size_t point_count;
};
BufferedPath bufferExtrusionPaths(const std::vector<PathPoint>& path_points) {
    BufferedPath result;
    result.point_count = path_points.size(); 

    glBindVertexArray(billboardVAO);

    // Create and bind the SSBO
    glGenBuffers(1, &result.path_buffer); checkGl();
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, result.path_buffer); checkGl();

    // Allocate memory for the SSBO
    glBufferData(GL_SHADER_STORAGE_BUFFER, path_points.size() * sizeof(PathPoint), path_points.data(), GL_STATIC_DRAW);
    checkGl();

    // Bind the SSBO to an indexed buffer binding point
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, result.path_buffer); checkGl();

    // Unbind the SSBO
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); checkGl();
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

void init()
{
    glGenVertexArrays(1, &billboardVAO);
    glBindVertexArray(billboardVAO);

    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

    checkGl();
    prepareData();

    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(int), vertexData.data(), GL_STATIC_DRAW);

    // vertex attributes - id:
    glEnableVertexAttribArray(vid_loc);
    glVertexAttribIPointer(vid_loc, 1, GL_INT, sizeof(int), (void *)0);
    checkGl();

    glBindVertexArray(0);
}

} // namespace billboard

#endif /* BILLBOARD_H_ */
