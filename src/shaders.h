#ifndef SHADERS_H_
#define SHADERS_H_

#include "globals.h"
#include <iostream>
#include <fstream>
#include <string>


namespace shaderProgram {
GLuint gcode_v_shader, gcode_f_shader, gcode_program;

GLuint visibility_v_shader, visibility_f_shader, visibility_program;

GLuint cheap_v_shader, cheap_f_shader, cheap_program;

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

void loadAndCompileShader(const std::string& source, GLuint& destination, GLenum type) {
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

static const std::string shaders_path = USE_HARDCODED_PATHS;

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

}

#endif /* SHADERS_H_ */
