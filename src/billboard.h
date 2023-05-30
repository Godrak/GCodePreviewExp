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

struct PathPoint
{
    glm::vec3 pos;
    glm::vec3 color;
    float height;
    float width;
};

struct BufferedPath
{
    GLuint pathTexture;
    size_t point_count;
};

BufferedPath bufferExtrusionPaths(const std::vector<PathPoint>& path_points) {
    BufferedPath result;
    result.point_count = path_points.size(); 

    glBindVertexArray(billboardVAO);

    // Create and bind the texture
    glGenTextures(1, &result.pathTexture); checkGl();
    glBindTexture(GL_TEXTURE_RECTANGLE, result.pathTexture); checkGl();

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST); checkGl();
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST); checkGl();

    std::cout << " size " << path_points.size() <<  std::endl;

    int maxTextureSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    std::cout << "max size is: "<< maxTextureSize << std::endl;

    // Allocate memory for the texture
    glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_R32F, 8, 31000, 0, GL_RED, GL_FLOAT, path_points.data());
    checkGl();

    // Bind the texture to an image unit
    glBindImageTexture(0, result.pathTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F); checkGl();

    // Unbind the texture
    glBindTexture(GL_TEXTURE_RECTANGLE, 0); checkGl();
    glBindVertexArray(0);

    return result;
}

BufferedPath generateTestingPathPoints()
{
    std::vector<PathPoint>                pathPoints;
    std::mt19937                          rng(std::random_device{}()); // Random number generator
    std::uniform_real_distribution<float> colorDist(0.0f, 1.0f);       // Random color distribution
    std::uniform_real_distribution<float> sizeDist(0.05f, 2.0f);        // Random size distribution
    std::uniform_real_distribution<float> diffDistance(-50.0f, 50.0f);   // Random size distribution

    glm::vec3 last_point = glm::vec3(diffDistance(rng), diffDistance(rng), 0); 
    for (int i = 0; i < 50; ++i) {
        PathPoint point;
        point.pos    = last_point + glm::vec3(diffDistance(rng), diffDistance(rng), 0);
        point.color  = glm::vec3(colorDist(rng), colorDist(rng), colorDist(rng)); // Random color
        point.height = sizeDist(rng);                                             // Random height between 1.0 and 5.0
        point.width  = 2.0*sizeDist(rng);                                             // Random width between 1.0 and 5.0
        last_point = point.pos;
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
