#include "core/Shader.hpp"
#include <GL/glew.h>
#include "core/error.hpp"
#include <iostream>

#include <glm/gtc/type_ptr.hpp>

Shader::Shader(int programId) { this->shader = programId; }

Shader::~Shader() { if(shader) Call(glDeleteProgram(shader)); }

void Shader::setUniformMat4fv(const std::string& name, const glm::mat4& matrix)
{
	bind();
	Call(glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix)));
}

void Shader::setIntArray(const std::string& name, std::vector<int> arr)
{
	bind();
	Call(glUniform1iv(GetUniformLocation(name), (GLsizei)arr.size(), arr.data()));
}

void Shader::bind() const { Call(glUseProgram(shader)); }

void Shader::setUniformVec4f(const std::string& name, const glm::vec4& v)
{
	bind();
	Call(glUniform4f(GetUniformLocation(name), v.x, v.y, v.z, v.w));
}

void Shader::setUniformVec3f(const std::string& name, const glm::vec3& v)
{
	bind();
	Call(glUniform3f(GetUniformLocation(name), v.x, v.y, v.z));
}

void Shader::setUniformVec3f(const std::string& name, const glm::vec4& v)
{
	bind();
	Call(glUniform3f(GetUniformLocation(name), v.x, v.y, v.z));
}

void Shader::setUniformVec2f(const std::string& name, const glm::vec2& v)
{
	bind();
	Call(glUniform2f(GetUniformLocation(name), v.x, v.y));
}

void Shader::setUniformVec2f(const std::string& name, const glm::vec4& v)
{
	bind();
	Call(glUniform2f(GetUniformLocation(name), v.x, v.y));
}

void Shader::setUniformVec2i(const std::string& name, int x, int y)
{
	bind();
	Call(glUniform2i(GetUniformLocation(name), x, y));
}

void Shader::setUniformFloat(const std::string& name, float v)
{
	bind();
	Call(glUniform1f(GetUniformLocation(name), v));
}

void Shader::setUniformInt(const std::string& name, int v)
{
	bind();
	Call(glUniform1i(GetUniformLocation(name), v));
}

void Shader::setUniformBool(const std::string& name, bool v)
{
	bind();
	Call(glUniform1i(GetUniformLocation(name), v));
}

int Shader::GetUniformLocation(const std::string& name)
{
	if (locationCache.find(name) != locationCache.end())
		return locationCache[name];
	int location = glGetUniformLocation(shader, name.c_str());
	locationCache[name] = location;
	if (location == -1)
		std::cout << "Warning: uniform '" << name << "' not found" << std::endl;
	return location;
}
