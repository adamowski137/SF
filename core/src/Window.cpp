#include "core/Window.hpp"
#include "core/InputHandler.hpp"
#include <iostream>
#include <stdexcept>
#include <glm/gtc/matrix_transform.hpp>

Window::Window(int width, int height, int ui_width, const char* title)
	: width(width), height(height), ui_width(ui_width)
{
	if (!glfwInit()) {
		throw std::runtime_error("Failed to initialize GLFW!");
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_STENCIL_BITS, 8);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	window = glfwCreateWindow(width, height, title, nullptr, nullptr);
	if (!window) {
		glfwTerminate();
		throw std::runtime_error("Failed to create GLFW window!");
	}

	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		std::cerr << "GLEW Error: " << glewGetErrorString(err) << std::endl;
		glfwDestroyWindow(window);
		glfwTerminate();
		throw std::runtime_error("Failed to initialize GLEW!");
	}

	int vpWidth = width - ui_width;
	if (vpWidth > 0 && height > 0)
		glViewport(0, 0, vpWidth, height);
}

Window::~Window()
{
	if (window) {
		glfwDestroyWindow(window);
		window = nullptr;
	}
	glfwTerminate();
}

bool Window::shouldClose() const
{
	return glfwWindowShouldClose(window);
}

bool Window::shouldDraw() const
{
	return !glfwGetWindowAttrib(window, GLFW_ICONIFIED);
}

void Window::finishFrame()
{
	glfwSwapBuffers(window);
	glfwPollEvents();
}

void Window::setupCallbacks(InputHandler& handler)
{
	glfwSetWindowUserPointer(window, &handler);
	glfwSetScrollCallback(window, InputHandler::scrollCallback);
	glfwSetCursorPosCallback(window, InputHandler::cursorPosCallback);

	handler.setWindowPointer(this);
	glfwSetFramebufferSizeCallback(window, InputHandler::framebufferSizeCallback);
}

void Window::handleWindowResize(int w, int h)
{
	this->width = w;
	this->height = h;
	int vpWidth = w - ui_width;
	if (vpWidth > 0 && h > 0)
		glViewport(0, 0, vpWidth, h);
}

glm::mat4 Window::getProjectionMatrix() const
{
	int vpWidth = getViewportWidth();
	float aspect = (height > 0 && vpWidth > 0) ? (float)vpWidth / (float)height : 1.0f;
	return glm::perspectiveRH(3.14159265f / 3.0f, aspect, 0.1f, 1000.f);
}

glm::mat4 Window::getInverseProjectionMatrix() const
{
	return glm::inverse(getProjectionMatrix());
}
