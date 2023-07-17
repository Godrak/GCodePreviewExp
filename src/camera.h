#ifndef CAMERA_H_
#define CAMERA_H_

#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include "globals.h"
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>


static const glm::vec3 UP = { 0, 0, 1 };

struct Camera
{
    glm::vec3 position      = {0, 10, 10};
    glm::vec3 target        = {0, 0, 0};
    glm::vec3 up            = {0, 0, 1};
    float     stepSize      = 0.5;
    float     rotationSpeed = 2;
    float     zoomSpeed     = 4;


	glm::mat4x4 get_view_projection() const {
		 auto view =  glm::lookAtRH(position, target, up);
		 auto perspective = glm::perspective<float>(glm::radians(45.0f),
                                                          (float) globals::screenResolution.x / (float) globals::screenResolution.y, 1.0f,
                                                          1000.0f);
		return perspective * view;
	}

    void moveCamera(glm::vec3 movements_vector)
    {
        glm::vec3 direction = glm::normalize(target - position);
        position += movements_vector.x * direction * stepSize;
        position += movements_vector.y * glm::normalize(glm::cross(direction, up)) * stepSize;
        position += movements_vector.z * glm::normalize(up) * stepSize;

        target += movements_vector.x * direction * stepSize;
        target += movements_vector.y * glm::normalize(glm::cross(direction, up)) * stepSize;
        target += movements_vector.z * glm::normalize(up) * stepSize;

        up = glm::cross(glm::cross(direction, UP), direction);
    }

    void rotateCamera(glm::vec2 offset)
    {
        glm::vec3 direction = glm::normalize(target - position);
        auto      step      = glm::normalize(glm::cross(direction, up));
        step *= offset[0];
        position += step;
        step = up * offset[1];
        position += step;

        up = glm::cross(glm::cross(direction, UP), direction);
    }

    void zoomCamera(double offset)
    {
        double max_dist = 1200;
        double min_dist = 10;

        double current = glm::distance(position, target);

        double target_dist = std::clamp(current + offset * zoomSpeed, min_dist, max_dist);
        position           = target + float(target_dist) * glm::normalize(position - target);
    }
};

#endif /* CAMERA_H_ */
