#pragma once

#include <unordered_map>
#include <string>
#include <vector>

#include <glm/glm.hpp>

class Shader {
	int shader;
	std::unordered_map<std::string, int> locationCache;
	int GetUniformLocation(const std::string& name);
public:
	Shader() = delete;
	Shader(int programId);
	~Shader();
	void bind() const;
	void setUniformVec4f(const std::string& name, const glm::vec4& v);
	void setUniformVec3f(const std::string& name, const glm::vec3& v);
	void setUniformVec3f(const std::string& name, const glm::vec4& v);
	void setUniformVec2f(const std::string& name, const glm::vec2& v);
	void setUniformVec2f(const std::string& name, const glm::vec4& v);
	void setUniformVec2i(const std::string& name, int x, int y);
	void setUniformFloat(const std::string& name, float v);
	void setUniformInt(const std::string& name, int v);
	void setUniformBool(const std::string& name, bool v);
	void setUniformMat4fv(const std::string& name, const glm::mat4& matrix);
	void setIntArray(const std::string& name, std::vector<int> arr);
};
