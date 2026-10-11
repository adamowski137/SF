#pragma once
#include <unordered_map>
#include "core/Shader.hpp"
#include <string>
#include <memory>

class ShaderManager
{
	std::unordered_map<std::string, std::shared_ptr<Shader>> shaders;
	std::vector<std::shared_ptr<Shader>> entityShaders;
public:
	void add(const std::string& name, const std::shared_ptr<Shader>& shader, bool isSelectable = true);
	std::shared_ptr<Shader> get(const std::string& name) const;
	void setColor(const Vector4<float>& color);
	std::unordered_map<std::string, std::shared_ptr<Shader>>& getShaders();
};
