#ifndef GLOBALS_H_
#define GLOBALS_H_

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <iostream>

namespace globals {
glm::ivec2 screenResolution = {512,512};
// I hoped that vis resolution could be lower, to reduce the rasterization load. Sadly, it the flickering of lines on and off is then much much worse
glm::ivec2 visibilityResolution = screenResolution;
}

namespace config {
bool updateCameraPosition = true;
bool geometryMode = false;
size_t total_segments_count = 0;
size_t visible_segments_count = 0;
bool with_visibility_pass = true;
bool vsync = true;
int visualization_type = 0;
bool color_update_required = true;
bool window_minimized = false;

}

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
