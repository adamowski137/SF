#include "core/App.hpp"
#include "core/MeshGenerator.hpp"
#include <iostream>

GLFWApplication::GLFWApplication()
{
	// glfwInit is called in Window constructor
}

GLFWApplication::~GLFWApplication()
{
}

App::App(int width, int height, int ui_width)
	: window(width, height, ui_width, "SF - Green Cube Scene"), ui(ui_width)
{
	std::cout << "OpenGL version: " << glGetString(GL_VERSION) << "\n";

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	int vpWidth = width - ui_width;
	if (vpWidth > 0 && height > 0)
		glViewport(0, 0, vpWidth, height);

	window.setupCallbacks(inputHandler);
	ui.initImGui(window.getNativeWindow());

	// Initial camera orientation
	camera.rotate(-3.14159265f / 4.0f, -3.14159265f / 6.0f);

	renderer = std::make_shared<Renderer>();
	cubeMesh = MeshGenerator::GetCubeMesh();

	glClearColor(0.12f, 0.12f, 0.14f, 1.0f);
}

App::~App()
{
}

void App::loop()
{
	while (!window.shouldClose())
	{
		processInput();
		render();
		window.finishFrame();
	}
}

void App::render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (window.shouldDraw())
	{
		ui.startImGuiFrame();

		renderer->setupCommonUniforms(camera, window);
		renderer->setLighting(
			Vector4<float>(lightPos[0], lightPos[1], lightPos[2], 1.0f),
			camera.getPosition()
		);

		// Render the green cube with Phong lighting
		Matrix4x4<float> model = Matrix4x4<float>::scale(cubeSize, cubeSize, cubeSize);
		Vector4<float> color(cubeColor[0], cubeColor[1], cubeColor[2], 1.0f);
		renderer->drawMeshPhong(cubeMesh, model, color);

		// Render UI
		ui.renderSimpleSceneUI(window, cubeSize, cubeColor, lightPos);
		ui.endImGuiFrame();
	}
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
