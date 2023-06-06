#ifndef SHADERS_H_
#define SHADERS_H_

#include "globals.h"

#include <iostream>
#include <fstream>
#include <string>

namespace shaderProgram {
GLuint gcode_v_shader, gcode_f_shader, gcode_program;

GLuint visibility_v_shader, visibility_f_shader, visibility_program;

GLuint quad_v_shader, quad_f_shader, quad_program;

bool check_shader(std::string source, GLuint id, GLenum st) {
	GLint logLength;
	glGetShaderiv(id, GL_INFO_LOG_LENGTH, &logLength);
	if (logLength > 2) {
		GLchar *log = new GLchar[logLength];
		glGetShaderInfoLog(id, logLength, &logLength, log);
		log[logLength - 1] = (GLchar) 0;
		std::cout << source << std::endl;
		std::cout << "Shader message: " << log << std::endl;

		delete[] log;
	}

	GLint status;
	glGetShaderiv(id, st, &status);
	return (status == GL_TRUE);
}

bool check_program(GLuint id, GLenum st) {
	GLint logLength;
	glGetProgramiv(id, GL_INFO_LOG_LENGTH, &logLength);
	if (logLength > 2) {
		GLchar *log = new GLchar[logLength];
		glGetProgramInfoLog(id, logLength, &logLength, log);
		log[logLength - 1] = (GLchar) 0;
		std::cout << "Program message: " << log << std::endl;
		delete[] log;
	}

	GLint status;
	glGetProgramiv(id, st, &status);
	return (status == GL_TRUE);
}

void loadAndCompileShader(std::string source, GLuint& destination, GLenum type) {
	std::ifstream shaderSource(source);
	std::string shaderCode;
	getline(shaderSource, shaderCode, (char) shaderSource.eof());
	const char *shaderCodeCString = shaderCode.data();

	destination = glCreateShader(type);
	glShaderSource(destination, 1, &shaderCodeCString, NULL);
	glCompileShader(destination);
	if (!check_shader(source, destination, GL_COMPILE_STATUS))
		exit(-1);
}

#define USE_HARDCODED_PATHS 1
#if USE_HARDCODED_PATHS
static const std::string shaders_path = "C:/prusa/references/GCodePreviewExp/win_glad/bin/";
#else
static const std::string shaders_path = "";
#endif // USE_HARDCODED_PATHS

void createGCodeProgram() {
	loadAndCompileShader(shaders_path + "gcode_shaders/v_shader.glsl", gcode_v_shader,
	GL_VERTEX_SHADER);
	loadAndCompileShader(shaders_path + "gcode_shaders/f_shader.glsl", gcode_f_shader,
	GL_FRAGMENT_SHADER);

	gcode_program = glCreateProgram();
	glAttachShader(gcode_program, gcode_v_shader);
	glAttachShader(gcode_program, gcode_f_shader);
	glLinkProgram(gcode_program);
	if (!check_program(gcode_program, GL_LINK_STATUS))
		exit(-1);
}

void createVisibilityProgram() {
	loadAndCompileShader(shaders_path + "gcode_shaders/v_shader.glsl", visibility_v_shader,
	GL_VERTEX_SHADER);
	loadAndCompileShader(shaders_path + "gcode_shaders/vis_f_shader.glsl", visibility_f_shader,
	GL_FRAGMENT_SHADER);

	visibility_program = glCreateProgram();
	glAttachShader(visibility_program, visibility_v_shader);
	glAttachShader(visibility_program, visibility_f_shader);
	glLinkProgram(visibility_program);
	if (!check_program(visibility_program, GL_LINK_STATUS))
		exit(-1);
}


void createQuadProgram() {
	loadAndCompileShader(shaders_path + "gcode_shaders/quad_v_shader.glsl", quad_v_shader,
	GL_VERTEX_SHADER);
	loadAndCompileShader(shaders_path + "gcode_shaders/quad_f_shader.glsl", quad_f_shader,
	GL_FRAGMENT_SHADER);

	quad_program = glCreateProgram();
	glAttachShader(quad_program, quad_v_shader);
	glAttachShader(quad_program, quad_f_shader);
	glLinkProgram(quad_program);
	if (!check_program(quad_program, GL_LINK_STATUS))
		exit(-1);
}

}

#endif /* SHADERS_H_ */
