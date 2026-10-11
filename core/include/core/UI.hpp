#pragma once

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>

class Window;
class InputHandler;

class UI {
protected:
	bool imGuiInitialized = false;
	int width;

public:
	UI(int width);
	virtual ~UI();

	void initImGui(GLFWwindow* window);
	void startImGuiFrame();
	void endImGuiFrame();

	// Pure virtual or customizable method for derived project UIs
	virtual void renderUI(Window& window);

	static void renderSepAndSpace();
	int getWidth() const { return width; }
};
