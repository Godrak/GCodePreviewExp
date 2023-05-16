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
    vertexData = std::vector<int>{0, 1, 2, 2, 3, 0};
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

BufferedPath generateTestingPathPoints()
{
    std::vector<PathPoint>                pathPoints;
    std::mt19937                          rng(std::random_device{}()); // Random number generator
    std::uniform_real_distribution<float> colorDist(0.0f, 1.0f);       // Random color distribution
    std::uniform_real_distribution<float> sizeDist(1.0f, 5.0f);        // Random size distribution
    std::uniform_real_distribution<float> diffDistance(1.0f, 50.0f);   // Random size distribution

    glm::vec3 last_point = glm::vec3(diffDistance(rng), diffDistance(rng), diffDistance(rng)); 
    for (int i = 0; i < 10; ++i) {
        PathPoint point;
        point.pos    = last_point + glm::vec3(diffDistance(rng), diffDistance(rng), diffDistance(rng));
        point.color  = glm::vec3(colorDist(rng), colorDist(rng), colorDist(rng)); // Random color
        point.height = sizeDist(rng);                                             // Random height between 1.0 and 5.0
        point.width  = sizeDist(rng);                                             // Random width between 1.0 and 5.0
        last_point = point.pos;
        pathPoints.push_back(point);
    }

    BufferedPath result;
    result.point_count = pathPoints.size(); 

    glBindVertexArray(billboardVAO);

    // Create and bind the texture
    glGenTextures(1, &result.pathTexture); checkGl();
    glBindTexture(GL_TEXTURE_RECTANGLE, result.pathTexture); checkGl();

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST); checkGl();
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST); checkGl();

    // Allocate memory for the texture
    glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_R32F, pathPoints.size(), 8, 0, GL_RED, GL_FLOAT, pathPoints.data()); checkGl();

    // Bind the texture to an image unit
    glBindImageTexture(0, result.pathTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F); checkGl();

    // Unbind the texture
    glBindTexture(GL_TEXTURE_RECTANGLE, 0); checkGl();
    glBindVertexArray(0);

    return result;
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
