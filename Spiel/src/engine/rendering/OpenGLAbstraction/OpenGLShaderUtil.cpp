#include "OpenGLShaderUtil.hpp"

#include <iostream>
#include <fstream>
#include <optional>
#include <cassert>

#include "GL/glew.h"
#include "GLFW/glfw3.h"

std::string readShader(std::string const& path)
{
	std::ifstream ifs(path);
	if (!ifs.good()) {
		std::cerr << "error: could not read shader from path: " << path << std::endl;
		return "error could not compile";
	}

	std::string res;
	std::string line;
	while (getline(ifs, line)) {
		res.append(line).append("\n");
	}
	return res;
}

u32 compileOGLShader(unsigned int type_, const std::string const& source)
{
	GLuint id = glCreateShader(type_);
	char const* src = source.c_str();
	glShaderSource(id, 1, &src, nullptr);
	glCompileShader(id);

	int result;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE) {
		int length;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
		char* message = (char*)_malloca(length * sizeof(char));
		glGetShaderInfoLog(id, length, &length, message);
		std::cerr << "error: failed to compile shader: " << message << std::endl;
		glDeleteShader(id);
		return 0;
	}

	return id;
}

u32 createOGLShaderProgram(std::string const& vertexShader, std::string const& fragmentShader)
{
	GLuint program = glCreateProgram();
	GLuint vs = compileOGLShader(GL_VERTEX_SHADER, vertexShader);
	GLuint fs = compileOGLShader(GL_FRAGMENT_SHADER, fragmentShader);

	assert(vs != 0);
	assert(fs != 0);

	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glValidateProgram(program);

	glDeleteShader(vs);
	glDeleteShader(fs);
	return program;
}