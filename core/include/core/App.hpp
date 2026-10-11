#pragma once
#include <GL/glew.h>
#include "core/Window.hpp"
#include "core/InputHandler.hpp"
#include "core/Renderer.hpp"
#include "core/Camera.hpp"
#include "core/UI.hpp"
#include <memory>
#include <string>

class GLFWApplication {
protected:
	GLFWApplication();
	virtual ~GLFWApplication();
};

class App : public GLFWApplication {
protected:
	Window window;
	InputHandler inputHandler;
	std::unique_ptr<UI> ui;
	Camera camera;
	std::shared_ptr<Renderer> renderer;

	float cameraRotSpeed = 0.02f;
	float cameraMoveSpeed = 0.005f;

public:
	App(int width, int height, int ui_width = 300, const std::string& title = "SF Application");
	virtual ~App();

	void loop();

protected:
	virtual void init();
	virtual void update(float dt);
	virtual void render();
	virtual void renderScene();
	virtual void processInput();
};
