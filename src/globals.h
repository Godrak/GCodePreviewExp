#ifndef GLOBALS_H_
#define GLOBALS_H_

#include <glm/glm.hpp>

namespace globals {

glm::ivec2 screenResolution = {1600,830};

GLint mvp_location = 0;
GLint camera_position_location = 1;
GLint screen_size_location = 2;
}

namespace config {
const int unitsPerMeter = 100;
const float verticesPerMeter = unitsPerMeter * 0.003; //0.3
const glm::vec2 terrainSizeM = { 300, 300 };
const glm::vec2 terrainSizeU = { terrainSizeM.x * unitsPerMeter, terrainSizeM.y
		* unitsPerMeter };

bool updateCameraPosition = true;
bool geometryMode = false;
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
