#pragma once
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include "core/ShaderManager.hpp"
#include "core/Mesh.hpp"

class Camera;
class Window;

class Renderer {
	const float DEF_LINE_WIDTH = 1.f;

	ShaderManager shaderManager;
	void initShaders();
	void addShader(const std::string& name, const std::string& vsPath, 
		const std::string& psPath, const std::string& gsPath = "", const std::pair<std::string, std::string>& tessPath = {"", ""},
		bool useForEntities = true);
public:
	Renderer();
	void setupCommonUniforms(Camera& camera, Window& window);
	void setLighting(const Vector4<float>& lightPos, const Vector4<float>& cameraPos);
	void drawLines(const std::shared_ptr<Mesh>& mesh_);
	void drawMeshPhong(const std::shared_ptr<Mesh>& mesh_, const Matrix4x4<float>& modelMatrix, const Vector4<float>& color);
};
