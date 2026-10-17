#include "core/Renderer.hpp"
#include "core/Shader.hpp"
#include "core/ShaderBuilder.hpp"
#include "core/error.hpp"
#include "core/Camera.hpp"
#include "core/Window.hpp"

void Renderer::initShaders()
{
	this->addShader("default", "res/shaders/simpleVertex.glsl", "res/shaders/simpleFragment.glsl");
	this->addShader("phong", "res/shaders/phongVertex.glsl", "res/shaders/phongFragment.glsl");
}

void Renderer::addShader(const std::string& name, const std::string& vsPath,
	const std::string& psPath, const std::string& gsPath, const std::pair<std::string, std::string>& tessPath,
	bool usedForEntities)
{
	ShaderBuilder builder;
	builder.addVertexShader(vsPath.c_str());
	builder.addFragmentShader(psPath.c_str());
	if (!gsPath.empty())
		builder.addGeometryShader(gsPath.c_str());
	if (!tessPath.first.empty()) {
		builder.addTesselationControlShader(tessPath.first.c_str());
		builder.addTesselationEvalShader(tessPath.second.c_str());
	}
	shaderManager.add(name, builder.compileAndGetShaderProgram(), usedForEntities);
}

void Renderer::setupCommonUniforms(Camera& camera, Window& window)
{
	for (const auto& shader : shaderManager.getShaders())
	{
		shader.second->setUniformMat4fv("projection", window.getProjectionMatrix());
		shader.second->setUniformMat4fv("view", camera.getViewMatrix());
	}
}

void Renderer::setLighting(const glm::vec3& lightPos, const glm::vec3& cameraPos)
{
	auto phong = shaderManager.get("phong");
	if (phong) {
		phong->setUniformVec3f("lightPos", lightPos);
		phong->setUniformVec3f("cameraPos", cameraPos);
	}
}

void Renderer::setLighting(const glm::vec4& lightPos, const glm::vec4& cameraPos)
{
	auto phong = shaderManager.get("phong");
	if (phong) {
		phong->setUniformVec3f("lightPos", lightPos);
		phong->setUniformVec3f("cameraPos", cameraPos);
	}
}

Renderer::Renderer()
{
	this->initShaders();
	glLineWidth(DEF_LINE_WIDTH);
}

void Renderer::drawLines(const std::shared_ptr<Mesh>& mesh_)
{
	shaderManager.get("default")->bind();
	shaderManager.get("default")->setUniformMat4fv("model", glm::mat4(1.0f));
	shaderManager.get("default")->setUniformVec4f("color", glm::vec4(0.5f, 0.f, 0.5f, 1.0f));
	glLineWidth(3.f);
	mesh_->bind();
	Call(glDrawElements(GL_LINES, mesh_->getIndiciesCount(), GL_UNSIGNED_INT, nullptr));
}

void Renderer::drawMeshPhong(const std::shared_ptr<Mesh>& mesh_, const glm::mat4& modelMatrix, const glm::vec4& color)
{
	shaderManager.get("phong")->bind();
	shaderManager.get("phong")->setUniformMat4fv("model", modelMatrix);
	shaderManager.get("phong")->setUniformVec4f("color", color);
	mesh_->bind();
	Call(glDrawElements(GL_TRIANGLES, mesh_->getIndiciesCount(), GL_UNSIGNED_INT, nullptr));
}
