#include "SpringSimApp.hpp"
#include "SpringSimUI.hpp"
#include "core/MeshGenerator.hpp"
#include <matrix/matrix.hpp>

SpringSimApp::SpringSimApp(int width, int height, int ui_width)
	: App(width, height, ui_width, "SF - 01 Spring Simulation")
{
	// Replace default UI with project-specific SpringSimUI
	ui = std::make_unique<SpringSimUI>(ui_width, cubeSize, cubeColor, lightPos);
	ui->initImGui(window.getNativeWindow());
}

void SpringSimApp::init()
{
	cubeMesh = MeshGenerator::GetCubeMesh();
}

void SpringSimApp::renderScene()
{
	renderer->setLighting(
		Vector4<float>(lightPos[0], lightPos[1], lightPos[2], 1.0f),
		camera.getPosition()
	);

	// Render the green cube with Phong lighting
	Matrix4x4<float> model = Matrix4x4<float>::scale(cubeSize, cubeSize, cubeSize);
	Vector4<float> color(cubeColor[0], cubeColor[1], cubeColor[2], 1.0f);
	renderer->drawMeshPhong(cubeMesh, model, color);
}
