/*
 * skybox.h
 *
 *  Created on: Jun 9, 2019
 *      Author: Pavel Mikus
 *		mail: pavel.mikus@eyen.se
 */

#ifndef SKYBOX_H_
#define SKYBOX_H_

#include <epoxy/gl_generated.h>
#include <glm/glm.hpp>
#include <vector>

namespace skybox {
GLuint skyboxVAO, vertexBuffer, elementBuffer;
GLint vpos_location = 0;
GLint vcol_location = 1;

struct vertex {
	glm::vec3 position;

	vertex(glm::vec3 pos) :
			position(pos) {
	}
};

std::vector<vertex> vertexData;
std::vector<uint> indicesData;

void prepareData() {
	float size = 500;
	vertexData = std::vector<vertex> {
			// front
			{ { -size, -size, size } },
			{ { size, -size, size } },
			{ { size, size, size } },
			{ { -size, size, size } },
			// back
			{ { -size, -size, -size } },
			{ { size, -size, -size } },
			{ { size, size, -size } },
			{ { -size, size, -size } }
	};

	indicesData = std::vector<uint> {
			// front
			0, 1, 2,
			2, 3, 0,
			// right
			1, 5, 6,
			6, 2, 1,
			// back
			7, 6, 5,
			5, 4, 7,
			// left
			4, 0, 3,
			3, 7, 4,
			// bottom
			4, 5, 1,
			1, 0, 4,
			// top
			3, 2, 6,
			6, 7, 3
	};
}

void init() {
	glGenVertexArrays(1, &skyboxVAO);
	glBindVertexArray(skyboxVAO);

	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

	glGenBuffers(1, &elementBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);

	checkGl();

	prepareData();
	checkGl();

	glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(vertex),
			vertexData.data(), GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesData.size() * sizeof(uint),
			indicesData.data(), GL_STATIC_DRAW);

// vertex attributes - position:
	glEnableVertexAttribArray(vpos_location);
	glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE, sizeof(vertex),
			(void *) 0);
	checkGl();

	glBindVertexArray(0);
}

}

#endif /* SKYBOX_H_ */
