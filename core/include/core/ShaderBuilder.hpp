#pragma once
#include <GL/glew.h>
#include "core/Shader.hpp"
#include <vector>
#include <memory>

class ShaderBuilder {
	std::vector<unsigned> shaders;
	std::string readFileContents(const char* filepath);
	unsigned createShader(const char* filepath, GLenum type);
public:
	~ShaderBuilder();
	void addVertexShader(const char* filepath);
	void addGeometryShader(const char* filepath);
	void addTesselationControlShader(const char* filepath);
	void addTesselationEvalShader(const char* filepath);
	void addFragmentShader(const char* filepath);
	void addComputeShader(const char* filepath);

	std::shared_ptr<Shader> compileAndGetShaderProgram();
};
