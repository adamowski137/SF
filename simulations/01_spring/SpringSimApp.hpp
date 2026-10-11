#pragma once

#include "core/App.hpp"
#include "core/Mesh.hpp"
#include <memory>

class SpringSimApp : public App {
	std::shared_ptr<Mesh> cubeMesh;

	// Scene specific state
	float cubeSize = 1.5f;
	float cubeColor[3] = { 0.1f, 0.85f, 0.2f }; // Green cube
	float lightPos[3] = { 5.0f, 5.0f, 5.0f };

public:
	SpringSimApp(int width, int height, int ui_width = 300);
	~SpringSimApp() override = default;

protected:
	void init() override;
	void renderScene() override;
};
