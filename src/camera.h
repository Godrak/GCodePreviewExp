#ifndef CAMERA_H_
#define CAMERA_H_

#include "glm/fwd.hpp"
#include "globals.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>


static const glm::vec3 UP = { 0, 0, 1 };

struct Camera
{
    glm::vec3 position      = {0, 10, 10};
    glm::vec3 direction     = {0, -1, -1};
    glm::vec3 up            = {0, 0, 1};
    float     stepSize      = 0.5;
    float     rotationSpeed = 2;

    std::vector<char> movement_history{};
    std::vector<glm::vec2> rotation_history{};

    void clear_history() {
        movement_history.clear();
        rotation_history.clear();
    }

	glm::mat4x4 get_view_projection() const {
		 auto view =  glm::lookAtRH(position, position + direction, up);
		 auto perspective = glm::perspective<float>(glm::radians(45.0f),
                                                          (float) globals::screenResolution.x / (float) globals::screenResolution.y, 1.0f,
                                                          1000.0f);
		return perspective * view;
	}

    // std::pair<glm::mat4x4, glm::vec3> predict_view_projection_and_position() const
    // {
    //     float prediction_frames = 1; 
    //     Camera predicted_camera = *this;
    //     for (size_t i = 0; i < prediction_frames; i++) {
    //         for (char move : movement_history)
    //             predicted_camera.moveCamera(move);
    //     }

    //     return {predicted_camera.get_view_projection(), predicted_camera.position};
    // }

    void moveCamera(char c)
    {
        movement_history.push_back(c);
        switch (c) {
        case 'w': position += glm::normalize(direction) * stepSize; break;
        case 's': position -= glm::normalize(direction) * stepSize; break;
        case 'a': position -= glm::normalize(glm::cross(direction, up)) * stepSize; break;
        case 'd': position += glm::normalize(glm::cross(direction, up)) * stepSize; break;
        case 'q': position += glm::normalize(up) * stepSize; break;
        case 'z': position -= glm::normalize(up) * stepSize; break;
        default: break;
        }
        up = glm::cross(glm::cross(direction, UP), direction);
    }

    void moveCamera(glm::vec2 offset)
    {
        rotation_history.push_back(offset);
        auto step = glm::normalize(glm::cross(direction, up));
        step *= offset[0];
        direction = glm::normalize(direction - step);

        step      = up * offset[1];
        direction = glm::normalize(direction - step);

        up = glm::cross(glm::cross(direction, UP), direction);
    }
};

#endif /* CAMERA_H_ */
