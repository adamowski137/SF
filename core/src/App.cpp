#include "core/App.hpp"
#include <iostream>

GLFWApplication::GLFWApplication()
{
}

GLFWApplication::~GLFWApplication()
{
}

App::App(int width, int height, int ui_width, const std::string& title)
	: window(width, height, ui_width, title.c_str()), ui(std::make_unique<UI>(ui_width))
{
	std::cout << "OpenGL version: " << glGetString(GL_VERSION) << "\n";

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	int vpWidth = width - ui_width;
	if (vpWidth > 0 && height > 0)
		glViewport(0, 0, vpWidth, height);

	window.setupCallbacks(inputHandler);
	ui->initImGui(window.getNativeWindow());

	camera.rotate(-3.14159265f / 4.0f, -3.14159265f / 6.0f);
	renderer = std::make_shared<Renderer>();

	glClearColor(0.12f, 0.12f, 0.14f, 1.0f);
}

App::~App()
{
}

void App::init()
{
}

void App::update(float dt)
{
	(void)dt;
}

void App::loop()
{
	init();

	while (!window.shouldClose())
	{
		processInput();
		update(0.016f);
		render();
		window.finishFrame();
	}
}

void App::render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (window.shouldDraw())
	{
		renderer->setupCommonUniforms(camera, window);

		renderScene();

		if (ui)
		{
			ui->startImGuiFrame();
			ui->renderUI(window);
			ui->endImGuiFrame();
		}
	}
}

void App::renderScene()
{
	// Base implementation does nothing
}

void App::processInput()
{
	if (inputHandler.isPressed(ImGuiKey_MouseRight, true))
	{
		auto mouseMove = inputHandler.getMouseMove();
		if (inputHandler.isPressed(ImGuiKey_LeftShift, true))
		{
			camera.pan(mouseMove.first * cameraMoveSpeed, -mouseMove.second * cameraMoveSpeed);
		}
		else
		{
			camera.rotate(mouseMove.first * cameraRotSpeed, mouseMove.second * cameraRotSpeed);
		}
	}

	camera.zoom(static_cast<float>(inputHandler.getScrollMove()));
}
