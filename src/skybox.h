/*
 * skybox.h
 *
 *  Created on: Jun 9, 2019
 *      Author: Pavel Mikus
 *		mail: pavel.mikus@eyen.se
 */

#ifndef SKYBOX_H_
#define SKYBOX_H_

#include "globals.h"

#if !USE_GLAD
#include <epoxy/gl_generated.h>
#endif // !USE_GLAD
#include <glm/glm.hpp>
#include <vector>
#include "misc/sky.c"

namespace skybox {
GLuint skyboxVAO, vertexBuffer, elementBuffer;
GLuint skybox_tex;
GLint vpos_location = 0;
GLint vcol_location = 1;

struct vertex {
	glm::vec3 position;

	vertex(glm::vec3 pos) :
			position(pos) {
	}
};

std::vector<vertex> vertexData;
std::vector<uint8_t> indicesData;

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

	indicesData = std::vector<uint8_t> {
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

void addSkyPixel(int x, int y, std::vector<unsigned char>& result) {
	result.push_back(sky_image.pixel_data[y * sky_image.width * sky_image.bytes_per_pixel + x * sky_image.bytes_per_pixel]);
	result.push_back(sky_image.pixel_data[y * sky_image.width * sky_image.bytes_per_pixel + x * sky_image.bytes_per_pixel + 1]);
	result.push_back(sky_image.pixel_data[y * sky_image.width * sky_image.bytes_per_pixel + x * sky_image.bytes_per_pixel + 2]);
}

std::vector<unsigned char> getSkyboxData(GLuint direction, int& width) {
	auto recWidth = sky_image.width / 4;

	if (recWidth != sky_image.height / 3)
		std::cout << "WARNING: invalid skybox sizes" << std::endl;

	width = recWidth;
	glm::ivec2 recStart;

	switch (direction) {
	case GL_TEXTURE_CUBE_MAP_POSITIVE_X: //right
		recStart = {1,2};
		break;
		case GL_TEXTURE_CUBE_MAP_NEGATIVE_X: //	Left
		recStart = {1,0};
		break;
		case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:	//Top
		recStart = {1,1};
		break;
		case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:	//Bottom
		recStart= {3,1};
		break;
		case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:	//Back
		recStart = {0,1};
		break;
		case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:	//Front
		recStart = {2,1};
		break;
		default:
		std::cout << "ERROR: invalid skybox direction" << std::endl;
		break;
	}

	recStart *= recWidth;
	std::vector<unsigned char> result;

	switch (direction) {
	case GL_TEXTURE_CUBE_MAP_POSITIVE_X: //right
		for (int y = recStart.y; y < recStart.y + recWidth; ++y) {
			for (int x = recStart.x; x < recStart.x + recWidth; ++x) {
				addSkyPixel(x, y, result);
			}
		}
		break;
	case GL_TEXTURE_CUBE_MAP_NEGATIVE_X: //	Left
		for (int y = recStart.y + recWidth - 1; y >= recStart.y; --y) {
			for (int x = recStart.x + recWidth - 1; x >= recStart.x; --x) {
				addSkyPixel(x, y, result);
			}
		}
		break;
	case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:	//Top
		for (int x = recStart.x + recWidth - 1; x >= recStart.x; --x) {
			for (int y = recStart.y; y < recStart.y + recWidth; ++y) {
				addSkyPixel(x, y, result);
			}
		}
		break;
	case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:	//Bottom
		for (int x = recStart.x + recWidth - 1; x >= recStart.x; --x) {
					for (int y = recStart.y; y < recStart.y + recWidth; ++y) {
				addSkyPixel(x, y, result);
			}
		}
		for (int x = recStart.x; x < recStart.x + recWidth; ++x) {
			for (int y = recStart.y; y < recStart.y + recWidth; ++y) {
				addSkyPixel(x, y, result);
			}
		}
		break;
	case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:	//Back
		for (int x = recStart.x + recWidth - 1; x >= recStart.x; --x) {
			for (int y = recStart.y; y < recStart.y + recWidth; ++y) {
				addSkyPixel(x, y, result);
			}
		}
		break;
	case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:	//Front
		for (int x = recStart.x; x < recStart.x + recWidth; ++x) {
			for (int y = recStart.y + recWidth - 1; y >= recStart.y; --y) {
				addSkyPixel(x, y, result);
			}
		}
		break;
	default:
		std::cout << "ERROR: invalid skybox direction" << std::endl;
		break;
	}

	return result;
}

void initSkyboxTex() {
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	for (GLuint i = 0; i < 6; ++i) {
		int width;
		auto targetFace = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;
		auto data = getSkyboxData(targetFace, width);

		glTexImage2D(targetFace, 0, GL_RGB, width, width, 0,
				GL_RGB, GL_UNSIGNED_BYTE, data.data());
	}
}

void init() {
	glGenVertexArrays(1, &skyboxVAO);
	glBindVertexArray(skyboxVAO);

	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

	glGenBuffers(1, &elementBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);

	glActiveTexture (GL_TEXTURE0);
	glGenTextures(1, &skybox_tex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_tex);
	initSkyboxTex();
	checkGl();

	prepareData();
	checkGl();

	glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(vertex),
			vertexData.data(), GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesData.size() * sizeof(uint8_t),
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
