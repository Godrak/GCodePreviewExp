#ifndef GLOBALS_H_
#define GLOBALS_H_

#include <glm/glm.hpp>

namespace globals {

glm::ivec2 screenResolution = {1500,830};
// I hoped that vis resolution could be lower, to reduce the rasterization load. Sadly, it the flickering of lines on and off is then much much worse
glm::ivec2 visibilityResolution = screenResolution;

GLint vp_location = 0;
GLint camera_position_location = 1;
GLint visibility_pass_location = 2;
}

namespace config {
bool updateCameraPosition = true;
bool geometryMode = false;
float percentage_to_show = 1.0;
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
