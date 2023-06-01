
#include "epoxy/gl.h"
#include <epoxy/gl_generated.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <vector>
#include <iostream>

#define DEBUG

#include "globals.h"
#include "camera.h"
#include "shaders.h"
#include "gcode.h"

namespace fps {
static double fpsOrigin = 0.0;
static double fpsLastLog = 0.0;
static int frameCounter = 0;
const int FPS_MASK = 15;
const double EXP_FACTOR = 0.5;
const double FPS_MEASURE_INTERVAL = 0.5;
const double FPS_LOG_INTERVAL = 2.0;
static double fpsFps = 30.0;

static void fps_start() {
	fpsOrigin = fpsLastLog = glfwGetTime();
	fpsFps = 60.0;
	frameCounter = 0;
}

static void fps_frame(bool log = false) {
	if (!(++frameCounter & FPS_MASK))
		return;

	double current = glfwGetTime();
	if (current < fpsOrigin + FPS_MEASURE_INTERVAL)
		return;

	double currentFps = frameCounter / (current - fpsOrigin);
	fpsFps = fpsFps * EXP_FACTOR + currentFps * (1.0 - EXP_FACTOR);
	frameCounter = 0;
	fpsOrigin = current;

	if (log && current > fpsLastLog + FPS_LOG_INTERVAL) {
		std::cout << current << ": FPS: " << fpsFps << std::endl;
		fpsLastLog = current;
	}
}

}

namespace glfwContext {

double last_xpos, last_ypos;
GLFWwindow *window;
int vsync = 1;
char forth_back = ' ';
char left_right = ' ';
char up_down = ' ';

static void error_callback(int error, const char *description) {
	std::cout << "Error: " << description << std::endl;
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		switch (key) {
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, GLFW_TRUE);
			break;
		case GLFW_KEY_F2:
			config::geometryMode = 1 - config::geometryMode;
			break;
		case GLFW_KEY_F3:
			vsync = 1 - vsync;
			std::cout << "vsync: " << vsync << std::endl;
			glfwSwapInterval(vsync);
			break;
		case GLFW_KEY_R:
			config::percentage_to_show += 0.01;
			config::percentage_to_show = fmin(1.0f, config::percentage_to_show);
			break;
		case GLFW_KEY_F:
			config::percentage_to_show -= 0.01;
			config::percentage_to_show = fmax(0.0f, config::percentage_to_show);
			break;
		case GLFW_KEY_W:
			forth_back = ('w');
			break;

		case GLFW_KEY_S:
			forth_back = ('s');
			break;

		case GLFW_KEY_A:
			left_right = ('a');
			break;

		case GLFW_KEY_D:
			left_right = ('d');
			break;

		case GLFW_KEY_Q:
			up_down = ('q');
			break;

		case GLFW_KEY_E:
			up_down = ('e');
			break;
		case GLFW_KEY_KP_ADD:
			camera::stepSize += 2 / 60.0f;
			break;
		case GLFW_KEY_KP_SUBTRACT:
			camera::stepSize -= 2 / 60.0f;
			break;
		}
	}

	if (action == GLFW_RELEASE) {
		switch (key) {
		case GLFW_KEY_W:
		case GLFW_KEY_S:
			forth_back = ' ';
			break;
		case GLFW_KEY_A:
		case GLFW_KEY_D:
			left_right = ' ';
			break;
		case GLFW_KEY_Q:
		case GLFW_KEY_E:
			up_down = ' ';
			break;
		}
	}
}

static void cursor_position_callback(GLFWwindow *window, double xpos, double ypos) {
	glm::vec2 offset;
	offset[0] = -xpos + last_xpos;
	offset[1] = ypos - last_ypos;
	offset *= 0.001 * camera::rotationSpeed;
	camera::moveCamera(offset);

	last_xpos = xpos;
	last_ypos = ypos;
}

static void initGlfw() {
	glfwSetErrorCallback(error_callback);

	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(globals::screenResolution.x, globals::screenResolution.y, "ProjectZero", NULL, NULL);
	if (!window)
		exit(-1);

	glfwSetKeyCallback(window, key_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, cursor_position_callback);

	glfwMakeContextCurrent(window);
	glfwSwapInterval(vsync);

}

}

namespace rendering {

float lastTime;
float currentTime;
glm::vec3 lastCameraPosition = camera::position;

void switchConfiguration() {
	if (config::geometryMode) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);checkGl();
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);checkGl();
	}
}

void render(const gcode::BufferedPath& path) {
	currentTime = float(glfwGetTime());checkGl();
	auto delta = currentTime - lastTime;
	lastTime = currentTime;
	glfwGetFramebufferSize(glfwContext::window, &globals::screenResolution.x, &globals::screenResolution.y);checkGl();

	glViewport(0, 0, globals::screenResolution.x, globals::screenResolution.y);checkGl();

	camera::moveCamera(glfwContext::forth_back);checkGl();
	camera::moveCamera(glfwContext::up_down);checkGl();
	camera::moveCamera(glfwContext::left_right);checkGl();

	if (config::updateCameraPosition)
		lastCameraPosition = camera::position;

	glm::mat4x4 view_projection = glm::mat4x4(1.0);checkGl();
	camera::applyViewTransform(view_projection);
	camera::applyProjectionTransform(view_projection);




	glBindFramebuffer(GL_FRAMEBUFFER, 0);checkGl();
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);checkGl();
	glViewport(0, 0, globals::screenResolution.x, globals::screenResolution.y);checkGl();

    glBindVertexArray(gcode::gcodeVAO);
    glUseProgram(shaderProgram::gcode_program);
    checkGl();

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, path.path_buffer); // Bind the SSBO to the indexed buffer binding point 0
	
    glUniformMatrix4fv(globals::vp_location, 1, GL_FALSE, glm::value_ptr(view_projection));
    glUniform3fv(globals::camera_position_location, 1, glm::value_ptr(lastCameraPosition));
    checkGl();
    glDrawArraysInstanced(GL_TRIANGLES, 0, gcode::vertexData.size(), std::max(size_t(2),size_t((path.point_count-1) * config::percentage_to_show)));
    checkGl();

    glUseProgram(0);
    glBindVertexArray(0);

//SWAP BUFFERS
	glfwSwapBuffers(glfwContext::window);checkGl();
	glfwPollEvents();checkGl();
	switchConfiguration();
	fps::fps_frame(true);
}

void setup() {
	shaderProgram::creategcodeProgram();
	gcode::init();
	glClearColor(0.0,0.0,0.6,0.0);
	checkGl();
}

}

// Reader function to load vector of PathPoints from a file
std::vector<gcode::PathPoint> readPathPoints(const std::string& filename)
{
    std::vector<gcode::PathPoint> pathPoints;

    std::ifstream file(filename, std::ios::binary);
    if (!file)
    {
        std::cerr << "Error opening file: " << filename << std::endl;
        return pathPoints;
    }

    // Read the size of the vector
    size_t size;
    file.read(reinterpret_cast<char*>(&size), sizeof(size));
	std::cout << "SIZE IS: " << size << std::endl;
    pathPoints.resize(size);

    // Read each PathPoint object from the file
    for (auto& point : pathPoints)
    {
        file.read(reinterpret_cast<char*>(&point), sizeof(point));
    }

    file.close();

    return pathPoints;
}

int main(int argc, char* argv[]) {
    // Check if a filename argument is provided
    if (argc < 2) {
        std::cout << "Please provide a filename as an argument." << std::endl;
        return 1;
    }

    // Read the filename from argv[1]
    std::string filename = argv[1];

	auto points = readPathPoints(filename);

	glfwContext::initGlfw();
	rendering::lastTime = float(glfwGetTime());
	rendering::currentTime = float(glfwGetTime());

	rendering::setup();

	// gcode::BufferedPath path = gcode::generateTestingPathPoints();
	gcode::BufferedPath path = gcode::bufferExtrusionPaths(points);

	std::cout << "PATHS BUFFERED" << std::endl;

	fps::fps_start();
	while (!glfwWindowShouldClose(glfwContext::window)) {
		rendering::render(path);
	}

	glfwDestroyWindow(glfwContext::window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}
