#ifndef GLOBALS_H_
#define GLOBALS_H_

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

#include <iostream>
#include <algorithm>
#include "bitset"

namespace globals {
glm::ivec2 screenResolution = {512,512};
glm::ivec2 visibilityResolution = {512,512};
}

namespace config {
bool updateCameraPosition = true;
bool geometryMode = false;
bool with_visibility_pass = true;
bool vsync = true;
int visualization_type = 0;
bool ranges_update_required = true;
bool color_update_required = true;
bool camera_center_required = true;
bool use_travel_moves_data = true;
bool window_minimized = false;
bool force_full_model_render = false;
bool view_travel_paths = true;
bool view_perimeters = true;
bool view_inner_perimeters = true;
bool view_internal_infill = true;
bool view_solid_infills = true;
bool view_supports = true;
bool enabled_paths_update_required = true;

size_t camera_prediction_frames = 4;
size_t visiblity_multiframes_count = 10;

float voxel_size = 2;
}

class SequentialRange
{
		size_t m_global_min{ 0 };
		size_t m_global_max{ 0 };
		size_t m_current_min{ 0 };
		size_t m_current_max{ 0 };

public:
		void set_current_min(size_t min) {
				assert(min > 0);
				m_current_min = std::clamp(min - 1, m_global_min, m_global_max);
				if (m_current_max < m_current_min)
						m_current_max = m_current_min;
		}
		void set_current_max(size_t max) {
				assert(max > 0);
				m_current_max = std::clamp(max - 1, m_global_min, m_global_max);
				if (m_current_min > m_current_max)
						m_current_min = m_current_max;
		}
		void set_global_max(size_t max) {
				assert(max > 0);
				m_global_max = max - 1;
				assert(m_global_min <= m_global_max);
		}

		size_t get_current_size() const { return m_current_max - m_current_min; }

		size_t get_global_min() const { return m_global_min + 1; }
		size_t get_global_max() const { return m_global_max + 1; }
		size_t get_current_min() const { return m_current_min + 1; }
		size_t get_current_max() const { return m_current_max + 1; }

		void increase_current_min(size_t value) {
				m_current_min = (value + m_current_min > m_global_max) ? m_global_max : m_current_min + value;
				if (m_current_max < m_current_min)
						m_current_max = m_current_min;
		}
		void decrease_current_min(size_t value) {
				m_current_min = (value > m_current_min - m_global_min) ? m_global_min : m_current_min - value;
		}
		void increase_current_max(size_t value) {
				m_current_max = (value + m_current_max > m_global_max) ? m_global_max : m_current_max + value;
		}
		void decrease_current_max(size_t value) {
				m_current_max = (value > m_current_max - m_global_min) ? m_global_min : m_current_max - value;
				if (m_current_min > m_current_max)
						m_current_min = m_current_max;
		}
};

SequentialRange sequential_range;

static bool check_opengl() {
	GLenum err = glGetError();
	if (err == GL_NO_ERROR)
		return true;
	std::cout << "OpenGL message: " << (int) err << std::endl;
	return false;
}

static bool checkGLCall(GLenum err) {
	if (err == GL_NO_ERROR)
		return true;
	std::cout << "OpenGL message: " << (int) err << std::endl;
	return false;
}

static void checkGl() {
#ifdef DEBUG
	if (!check_opengl()) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
#endif
}

#endif /* GLOBALS_H_ */
