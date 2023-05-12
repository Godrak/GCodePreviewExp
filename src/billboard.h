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
#include <vector>
#include <random>

namespace billboard {
GLuint billboardVAO, vertexBuffer, elementBuffer;

GLuint pathSSBO;

GLuint pathSSBObindPoint = 0;

GLint vid_loc = 0;

struct vertex {
	glm::int8 vertex_id;

	vertex(glm::int8 id) :
			vertex_id(id) {
	}
};

std::vector<vertex> vertexData;
std::vector<uint> indicesData;

void prepareData() {
        vertexData = std::vector<vertex>{{0}, {1}, {2}, {3}};

        indicesData = std::vector<uint> {
			0, 1, 2,
			2, 3, 0
	};
}


struct PathPoint {
	glm::vec3 pos;
	glm::vec3 color;
	float height;
	float width;
};

std::vector<PathPoint> generateTestingPathPoints()
{
    std::vector<PathPoint> pathPoints;
    std::mt19937 rng(std::random_device{}()); // Random number generator
    std::uniform_real_distribution<float> colorDist(0.0f, 1.0f); // Random color distribution
    std::uniform_real_distribution<float> sizeDist(1.0f, 5.0f); // Random size distribution

    for (int i = 0; i < 10; ++i) {
        PathPoint point;
        point.pos = glm::vec3(i, i, i);
        point.color = glm::vec3(colorDist(rng), colorDist(rng), colorDist(rng)); // Random color
        point.height = sizeDist(rng); // Random height between 1.0 and 5.0
        point.width = sizeDist(rng); // Random width between 1.0 and 5.0
        pathPoints.push_back(point);
    }

    return pathPoints;
}

void createPathSSBO() {
	glGenBuffers(1, &pathSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, pathSSBO);

	auto pathPoints = generateTestingPathPoints();

	glBufferData(GL_SHADER_STORAGE_BUFFER, pathPoints.size() * sizeof(PathPoint), pathPoints.data(), GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, pathSSBObindPoint, pathSSBO);

	// Unbind the SSBO
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void init() {
	glGenVertexArrays(1, &billboardVAO);
	glBindVertexArray(billboardVAO);

	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

	glGenBuffers(1, &elementBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);

	checkGl();

	glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(vertex),
			vertexData.data(), GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesData.size() * sizeof(uint),
			indicesData.data(), GL_STATIC_DRAW);

// vertex attributes - id:
	glEnableVertexAttribArray(vid_loc);
	glVertexAttribPointer(vid_loc, 1, GL_BYTE, GL_FALSE, sizeof(vertex),
			(void *) 0);
	checkGl();

	glBindVertexArray(0);
}

}

#endif /* BILLBOARD_H_ */
