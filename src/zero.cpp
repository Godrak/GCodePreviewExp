
#include "epoxy/gl.h"
#include <GLFW/glfw3.h>
#include <epoxy/gl_generated.h>
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
#include "skybox.h"
#include "billboard.h"

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
			config::updateCameraPosition = 1 - config::updateCameraPosition;
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

void render(const billboard::BufferedPath& path) {
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

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	// glEnable(GL_CULL_FACE);
	// glCullFace(GL_FRONT);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);checkGl();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);checkGl();
	glViewport(0, 0, globals::screenResolution.x, globals::screenResolution.y);checkGl();

    //SKYBOX DRAW
	glUseProgram(shaderProgram::skybox_program);checkGl();
	glBindVertexArray(skybox::skyboxVAO);checkGl();

	glDisable(GL_DEPTH_TEST);checkGl();
	glDepthMask(false);checkGl();

	glUniformMatrix4fv(globals::vp_location, 1, GL_FALSE, glm::value_ptr(view_projection));checkGl();
	glUniform3fv(globals::camera_position_location, 1, glm::value_ptr(lastCameraPosition));checkGl();
	glUniform2iv(globals::screen_size_location, 1, glm::value_ptr(globals::screenResolution));checkGl();

	glDrawElements(GL_TRIANGLES, skybox::indicesData.size(), GL_UNSIGNED_INT, 0);checkGl();

	glEnable(GL_DEPTH_TEST);checkGl();
	glDepthMask(true);checkGl();

	glUseProgram(0);checkGl();
	glBindVertexArray(0);checkGl();


	// BILLBOARDED EXTRUSION PATHS
    glBindVertexArray(billboard::billboardVAO);
    glUseProgram(shaderProgram::billboard_program);
    checkGl();

    // Bind the texture to the texture unit 0
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_RECTANGLE, path.pathTexture);
	
    glUniformMatrix4fv(globals::vp_location, 1, GL_FALSE, glm::value_ptr(view_projection));
    glUniform3fv(globals::camera_position_location, 1, glm::value_ptr(lastCameraPosition));
    checkGl();
    glDrawArraysInstanced(GL_TRIANGLES, 0, billboard::vertexData.size(), path.point_count-1);
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
	shaderProgram::createSkyboxProgram();
	shaderProgram::createBillboardProgram();
	skybox::init();
	billboard::init();
	glClearColor(0.0,0.0,0.6,0.0);
	checkGl();
}

}

int main() {
	glfwContext::initGlfw();
	rendering::lastTime = float(glfwGetTime());
	rendering::currentTime = float(glfwGetTime());

	rendering::setup();

	billboard::BufferedPath path = billboard::generateTestingPathPoints();

	fps::fps_start();
	while (!glfwWindowShouldClose(glfwContext::window)) {
		rendering::render(path);
	}

	glfwDestroyWindow(glfwContext::window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}
