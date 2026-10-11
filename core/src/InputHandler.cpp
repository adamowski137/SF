#include "core/InputHandler.hpp"
#include "core/Window.hpp"
#include <algorithm>

void InputHandler::cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
	InputHandler* handler = static_cast<InputHandler*>(glfwGetWindowUserPointer(window));
	if (handler) {
		handler->mousedelta_x = xpos - handler->mousePos.first;
		handler->mousedelta_y = ypos - handler->mousePos.second;

		handler->mousePos = { static_cast<float>(xpos), static_cast<float>(ypos) };
	}
}

void InputHandler::scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	(void)xoffset;
	InputHandler* handler = static_cast<InputHandler*>(glfwGetWindowUserPointer(window));
	if (handler) {
		handler->scroll_delta += yoffset;
	}
}

void InputHandler::framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	InputHandler* handler = static_cast<InputHandler*>(glfwGetWindowUserPointer(window));
	if (handler && handler->window) {
		handler->window->handleWindowResize(width, height);
	}
}

bool InputHandler::isPressed(ImGuiKey key, bool held) const
{
	if (ImGui::GetIO().WantTextInput)
		return false;

	if (key == ImGuiKey_MouseLeft && ImGui::GetIO().WantCaptureMouse) {
		return false;
	}

	if (held)
		return ImGui::IsKeyDown(key);
	else
		return ImGui::IsKeyPressed(key);
}

void InputHandler::setWindowPointer(Window* win)
{
	this->window = win;
}

void InputHandler::resetMouseHoldPos()
{
	this->mouseHoldPos = this->mousePos;
}

std::pair<float, float> InputHandler::getMouseMove()
{
	auto delta = ImGui::GetIO().MouseDelta;
	return { delta.x, delta.y };
}

std::pair<float, float> InputHandler::getMousePos(bool normalized) const
{
	if (normalized && window)
	{
		std::pair<int, int> center = { (window->getViewportWidth() - 1) / 2, (window->getHeight() - 1) / 2 };
		int minCoordinate = std::min(window->getViewportWidth(), window->getHeight());
		if (minCoordinate <= 0) return { 0.f, 0.f };
		return { 2.f * (mousePos.first - center.first) / minCoordinate, -2.f * (mousePos.second - center.second) / minCoordinate };
	}
	else
		return mousePos;
}

std::pair<float, float> InputHandler::getMouseHoldPos(bool normalized) const
{
	if (normalized && window)
	{
		std::pair<int, int> center = { (window->getViewportWidth() - 1) / 2, (window->getHeight() - 1) / 2 };
		int minCoordinate = std::min(window->getViewportWidth(), window->getHeight());
		if (minCoordinate <= 0) return { 0.f, 0.f };
		return { 2.f * (mouseHoldPos.first - center.first) / minCoordinate, -2.f * (mouseHoldPos.second - center.second) / minCoordinate };
	}
	else
		return mouseHoldPos;
}

int InputHandler::getScrollMove()
{
	ImVec2 mPos = ImGui::GetMousePos();

	if (window) {
		int windowHeight = window->getHeight();
		int viewportWidth = window->getViewportWidth();

		bool isMouseInViewport =
			mPos.x >= 0 && mPos.x < viewportWidth &&
			mPos.y >= 0 && mPos.y < windowHeight;

		if (isMouseInViewport)
		{
			int delta = static_cast<int>(scroll_delta);
			scroll_delta = 0;
			return delta;
		}
	}

	scroll_delta = 0;
	return 0;
}
