#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <matrix/matrix.hpp>

class InputHandler;

class Window {
	GLFWwindow* window = nullptr;
	int width, height, ui_width;

public:
	Window(int width, int height, int ui_width = 300, const char* title = "SF Scene");
	~Window();

	// Prevent copying
	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;

	int getWidth() const { return width; }
	int getViewportWidth() const { return width - ui_width; }
	int getHeight() const { return height; }
	int getUiWidth() const { return ui_width; }
	void setUiWidth(int w) {
		ui_width = w;
		handleWindowResize(width, height);
	}

	bool shouldClose() const;
	bool shouldDraw() const;
	void finishFrame();

	void setupCallbacks(InputHandler& handler);
	void handleWindowResize(int width, int height);

	Matrix4x4<float> getProjectionMatrix() const;
	Matrix4x4<float> getInverseProjectionMatrix() const;

	GLFWwindow* getNativeWindow() const { return window; }
	GLFWwindow* getWindow() const { return window; }
};
