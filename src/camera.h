#ifndef CAMERA_H_
#define CAMERA_H_

#include "globals.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


static const glm::vec3 UP = { 0, 1, 0 };

namespace camera {
glm::vec3 position = { config::terrainSizeU.x/2, 15*config::unitsPerMeter, config::terrainSizeU.y/2 };
glm::vec3 direction = { 0, -1, 0.01};
glm::vec3 up = { 0, 1, 0 };
float stepSize = config::unitsPerMeter/3;
float rotationSpeed = 2;

void applyViewTransform(glm::mat4x4 &m) {
	m = glm::lookAt(position, position + direction, up)*m;
}

void applyProjectionTransform(glm::mat4x4 &m) {
	glm::mat4x4 perspective = glm::perspective<float>(glm::radians(60.0f),
			(float) globals::screenResolution.x / (float) globals::screenResolution.y, 100.0f, 30000.0f);
	m = perspective * m;
}

void moveCamera(char c) {
	switch (c) {
	case 'w':
		camera::position += glm::normalize(camera::direction) * stepSize;
		break;
	case 's':
		camera::position -= glm::normalize(camera::direction) * stepSize;
		break;
	case 'a':
		camera::position -= glm::normalize(
				glm::cross(camera::direction, camera::up)) * stepSize;
		break;
	case 'd':
		camera::position += glm::normalize(
				glm::cross(camera::direction, camera::up)) * stepSize;
		break;
	case 'q':
		camera::position += glm::normalize(up) * stepSize;
		break;
	case 'e':
		camera::position -= glm::normalize(up) * stepSize;
		break;
	default:
		break;
	}
	up = glm::cross(glm::cross(direction, UP), direction);
}

void moveCamera(glm::vec2 offset) {
	auto step = glm::normalize(glm::cross(direction, up));
	step *= offset[0];
	direction = glm::normalize(direction - step);

	step = up * offset[1];
	direction = glm::normalize(direction - step);

	up = glm::cross(glm::cross(direction, UP), direction);
}
}



#endif /* CAMERA_H_ */
