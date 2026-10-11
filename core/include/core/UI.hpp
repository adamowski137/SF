#pragma once

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>

class Window;
class InputHandler;

class UI {
	bool imGuiInitialized = false;
	int width;

public:
	UI(int width);
	~UI();

	void initImGui(GLFWwindow* window);
	void startImGuiFrame();
	void endImGuiFrame();

	void renderSimpleSceneUI(Window& window, float& cubeSize, float cubeColor[3], float lightPos[3]);

	void renderSepAndSpace();
};
