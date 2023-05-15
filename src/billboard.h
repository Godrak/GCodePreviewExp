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

namespace billboard {
GLuint billboardVAO, vertexBuffer;

GLuint pathSSBObindPoint = 5;

GLint vid_loc = 0;

std::vector<glm::int8> vertexData;

void prepareData()
{
    vertexData = std::vector<glm::int8>{0, 1, 2, 2, 3, 0};
}

struct PathPoint
{
    glm::vec3 pos;
    glm::vec3 color;
    float     height;
    float     width;
};

struct BufferedPath
{
    GLuint pathSSBO;
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
    glGenBuffers(1, &result.pathSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, result.pathSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, pathPoints.size() * sizeof(PathPoint), pathPoints.data(), GL_STATIC_DRAW);
    // Unbind the SSBO
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    return result;
}

void init()
{
    glGenVertexArrays(1, &billboardVAO);
    glBindVertexArray(billboardVAO);

    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

    checkGl();

    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(glm::int8), vertexData.data(), GL_STATIC_DRAW);

    // vertex attributes - id:
    glEnableVertexAttribArray(vid_loc);
    glVertexAttribPointer(vid_loc, 1, GL_BYTE, GL_FALSE, sizeof(glm::int8), (void *)0);
    checkGl();

    glBindVertexArray(0);
}

} // namespace billboard

#endif /* BILLBOARD_H_ */
