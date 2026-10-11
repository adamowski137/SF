#include "core/ShaderBuilder.hpp"
#include <sstream>
#include <fstream>
#include <iostream>
#include <algorithm>
#include "core/error.hpp"

std::string ShaderBuilder::readFileContents(const char* filepath)
{
	std::fstream ifs(filepath, std::ios::in);
	if (ifs.fail()) {
		std::cerr << "Error loading shader from file " << filepath << "\n";
		exit(1);
	}
	std::stringstream ss;
	ss << ifs.rdbuf();
	ifs.close();
	return ss.str();
}

unsigned ShaderBuilder::createShader(const char* filepath, GLenum type)
{
	unsigned int shader = Call(glCreateShader(type));
	std::string shader_code = readFileContents(filepath);
	const char* shader_code_cstr = shader_code.c_str();
	Call(glShaderSource(shader, 1, &shader_code_cstr, NULL));
	Call(glCompileShader(shader));
	int status;
	Call(glGetShaderiv(shader, GL_COMPILE_STATUS, &status));
	if (status == GL_FALSE)
	{
		GLint infoLogLength;
		Call(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength));
		GLchar* strInfoLog = new GLchar[infoLogLength + 1];
		Call(glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog));
		const char* strShaderType = NULL;
		switch (type) {
		case GL_VERTEX_SHADER: strShaderType = "vertex"; break;
		case GL_GEOMETRY_SHADER: strShaderType = "geometry"; break;
		case GL_FRAGMENT_SHADER: strShaderType = "fragment"; break;
		case GL_COMPUTE_SHADER: strShaderType = "compute"; break;
		}
		fprintf(stderr, "Compile failure in %s shader:\n%s\n", strShaderType, strInfoLog);
		delete[] strInfoLog;
	}
	return shader;
}

ShaderBuilder::~ShaderBuilder()
{
	std::for_each(shaders.begin(), shaders.end(), glDeleteShader);
}

void ShaderBuilder::addVertexShader(const char* filepath) { shaders.push_back(createShader(filepath, GL_VERTEX_SHADER)); }
void ShaderBuilder::addGeometryShader(const char* filepath) { shaders.push_back(createShader(filepath, GL_GEOMETRY_SHADER)); }
void ShaderBuilder::addTesselationControlShader(const char* filepath) { shaders.push_back(createShader(filepath, GL_TESS_CONTROL_SHADER)); }
void ShaderBuilder::addTesselationEvalShader(const char* filepath) { shaders.push_back(createShader(filepath, GL_TESS_EVALUATION_SHADER)); }
void ShaderBuilder::addFragmentShader(const char* filepath) { shaders.push_back(createShader(filepath, GL_FRAGMENT_SHADER)); }
void ShaderBuilder::addComputeShader(const char* filepath) { shaders.push_back(createShader(filepath, GL_COMPUTE_SHADER)); }

std::shared_ptr<Shader> ShaderBuilder::compileAndGetShaderProgram()
{
	unsigned int program = Call(glCreateProgram());
	for (auto q0 : shaders) Call(glAttachShader(program, q0));
	Call(glLinkProgram(program));
	GLint status;
	Call(glGetProgramiv(program, GL_LINK_STATUS, &status));
	if (status == GL_FALSE) {
		GLint infoLogLength;
		Call(glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength));
		GLchar* strInfoLog = new GLchar[infoLogLength + 1];
		Call(glGetProgramInfoLog(program, infoLogLength, NULL, strInfoLog));
		fprintf(stderr, "Linker failure: %s\n", strInfoLog);
		delete[] strInfoLog;
	}
	for (auto q0 : shaders) Call(glDetachShader(program, q0));
	return std::make_shared<Shader>(program);
}
