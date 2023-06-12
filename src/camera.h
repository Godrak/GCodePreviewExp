#ifndef CAMERA_H_
#define CAMERA_H_

#include "globals.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


static const glm::vec3 UP = { 0, 0, 1 };

namespace camera {
glm::vec3 position = { 0, 10, 10};
glm::vec3 direction = { 0, -1, -1};
glm::vec3 up = { 0, 0, 1 };
float stepSize = 0.5;
float rotationSpeed = 2;

void applyViewTransform(glm::mat4x4 &m) {
	m = glm::lookAtRH(position, position + direction, up)*m;
}

void applyProjectionTransform(glm::mat4x4 &m) {
	glm::mat4x4 perspective = glm::perspective<float>(glm::radians(45.0f),
			(float) globals::screenResolution.x / (float) globals::screenResolution.y, 1.0f, 1000.0f);
	m = perspective * m;
}

//Has higher FOV
void applyVisibilityProjectionTransform(glm::mat4x4 &m) {
	glm::mat4x4 perspective = glm::perspective<float>(glm::radians(60.0f),
			(float) globals::visibilityResolution.x / (float) globals::visibilityResolution.y, 1.0f, 1000.0f);
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
	case 'z':
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
