#ifndef SHADERS_H_
#define SHADERS_H_

#include <epoxy/gl.h>
#include <iostream>
#include <string>

#include "globals.h"

namespace shaderProgram {
GLuint skybox_v_shader, skybox_f_shader, skybox_program;

GLuint billboard_v_shader, billboard_f_shader, billboard_program;

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

void createSkyboxProgram() {
	loadAndCompileShader("skybox_shaders/v_shader.glsl", skybox_v_shader,
	GL_VERTEX_SHADER);
	loadAndCompileShader("skybox_shaders/f_shader.glsl", skybox_f_shader,
	GL_FRAGMENT_SHADER);

	skybox_program = glCreateProgram();
	glAttachShader(skybox_program, skybox_v_shader);
	glAttachShader(skybox_program, skybox_f_shader);
	glLinkProgram(skybox_program);
	if (!check_program(skybox_program, GL_LINK_STATUS))
		exit(-1);
}

void createBillboardProgram() {
	loadAndCompileShader("billboard_shaders/v_shader.glsl", billboard_v_shader,
	GL_VERTEX_SHADER);
	loadAndCompileShader("billboard_shaders/f_shader.glsl", billboard_f_shader,
	GL_FRAGMENT_SHADER);

	billboard_program = glCreateProgram();
	glAttachShader(billboard_program, billboard_v_shader);
	glAttachShader(billboard_program, billboard_f_shader);
	glLinkProgram(billboard_program);
	if (!check_program(billboard_program, GL_LINK_STATUS))
		exit(-1);
}

}

#endif /* SHADERS_H_ */
