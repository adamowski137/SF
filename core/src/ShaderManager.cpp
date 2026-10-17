#include "core/ShaderManager.hpp"

void ShaderManager::add(const std::string& name, const std::shared_ptr<Shader>& shader, bool isSelectable)
{
	this->shaders.insert({ name, shader });
	if(isSelectable)
		this->entityShaders.push_back(shader);
}

std::shared_ptr<Shader> ShaderManager::get(const std::string& name) const
{
	auto it = this->shaders.find(name);
	return it == this->shaders.end() ? nullptr : it->second;
}

void ShaderManager::setColor(const glm::vec4& color)
{
	for (auto& shader : entityShaders)
		shader->setUniformVec4f("color", color);
}

std::unordered_map<std::string, std::shared_ptr<Shader>>& ShaderManager::getShaders()
{
	return this->shaders;
}
