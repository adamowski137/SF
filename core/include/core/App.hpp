#pragma once
#include <GL/glew.h>
#include "core/Window.hpp"
#include "core/InputHandler.hpp"
#include "core/Renderer.hpp"
#include "core/Mesh.hpp"
#include "core/Camera.hpp"
#include "core/UI.hpp"
#include <memory>

class GLFWApplication {
protected:
	GLFWApplication();
	virtual ~GLFWApplication();
};

class App : public GLFWApplication {
	Window window;
	InputHandler inputHandler;
	UI ui;
	Camera camera;
	std::shared_ptr<Renderer> renderer;
	std::shared_ptr<Mesh> cubeMesh;

	const float cameraRotSpeed = 0.02f;
	const float cameraMoveSpeed = 0.005f;

	// Scene state
	float cubeSize = 1.5f;
	float cubeColor[3] = { 0.1f, 0.85f, 0.2f }; // Green cube
	float lightPos[3] = { 5.0f, 5.0f, 5.0f };

public:
	App(int width, int height, int ui_width = 300);
	~App();

	void loop();
	void render();
	void processInput();
};
