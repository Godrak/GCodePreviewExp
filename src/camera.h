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
    glm::vec3 position{ -10.0f, -10.0f, 10.0f };
    glm::vec3 target{0, 0, 0};    
    glm::vec3 forward{ 0.577350259f, 0.577350259f, -0.577350259f };
    glm::vec3 up{ 0.408248335f, 0.408248335f, 0.816496670f };
    float     stepSize{ 0.5f };
    float     rotationSpeed{ 2.0f };
    float     zoomSpeed{ 4.0f };
    float     orthoZoom{ 0.25f };
    float     orthoZoomStepSize{ 0.001f };


	  glm::mat4x4 get_view_projection() const {
        auto view =  glm::lookAtRH(position, target, up);
        if (config::orthographic_camera) {
            const float half_w = orthoZoom * 0.5f * globals::screenResolution.x;
            const float half_h = orthoZoom * 0.5f * globals::screenResolution.y;
            const auto ortho = glm::ortho<float>(-half_w, half_w, -half_h, half_h, 1.0f, 1000.0f);
            return ortho * view;
        }
        else {
            const auto perspective = glm::perspective<float>(glm::radians(45.0f),
                (float)globals::screenResolution.x / (float)globals::screenResolution.y, 1.0f, 1000.0f);
            return perspective * view;
        }
    }

    void moveCamera(const glm::vec3& movements_vector)
    {
        glm::vec3 displacement = config::orthographic_camera ? glm::vec3(0.0f, 0.0f, 0.0f) : movements_vector.x * forward * stepSize;
        displacement += movements_vector.y * glm::normalize(glm::cross(forward, up)) * stepSize +
                        movements_vector.z * glm::normalize(up) * stepSize;
        position += displacement;
        target += displacement;

        if (movements_vector.x > 0.0f)
            orthoZoom *= (1.0f - orthoZoomStepSize);
        else if (movements_vector.x < 0.0f)
            orthoZoom *= (1.0f + orthoZoomStepSize);

        up = glm::cross(glm::cross(forward, UP), forward);
    }

    void rotateCamera(const glm::vec2& offset)
    {
        forward = glm::normalize(target - position);
        auto      step = glm::normalize(glm::cross(forward, up));
        step *= offset[0];
        position += step;
        step = up * offset[1];
        position += step;

        up = glm::cross(glm::cross(forward, UP), forward);
    }

    void zoomCamera(double offset)
    {
        const double max_dist = 1200;
        const double min_dist = 10;

        const double current = glm::distance(position, target);

        const double target_dist = std::clamp(current + offset * zoomSpeed, min_dist, max_dist);
        position = target - float(target_dist) * forward;

        if (offset > 0.0f)
            orthoZoom *= (1.0f + 12.5f * orthoZoomStepSize);
        else if (offset < 0.0f)
            orthoZoom *= (1.0f - 12.5f * orthoZoomStepSize);
    }
};

#endif /* CAMERA_H_ */
