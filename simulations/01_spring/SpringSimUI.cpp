#include "SpringSimUI.hpp"
#include "core/Window.hpp"

SpringSimUI::SpringSimUI(int width, float& inCubeSize, float* inCubeColor, float* inLightPos)
	: UI(width), cubeSize(inCubeSize), cubeColor(inCubeColor), lightPos(inLightPos)
{
}

void SpringSimUI::renderUI(Window& window)
{
	const ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse;

	ImGui::SetNextWindowPos(ImVec2(static_cast<float>(window.getViewportWidth()), 0.0f));
	ImGui::SetNextWindowSize(ImVec2(static_cast<float>(window.getWidth() - window.getViewportWidth()), static_cast<float>(window.getHeight())));

	ImGui::Begin("01_Spring Controls", nullptr, windowFlags);
	ImGui::Dummy(ImVec2(0.f, 10.f));

	ImGui::Text("Spring Simulation - 01");
	ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

	renderSepAndSpace();

	ImGui::Text("Cube Properties");
	ImGui::SliderFloat("Size", &cubeSize, 0.1f, 5.0f);
	ImGui::ColorEdit3("Color", cubeColor);

	renderSepAndSpace();

	ImGui::Text("Lighting");
	ImGui::SliderFloat3("Light Position", lightPos, -20.0f, 20.0f);

	renderSepAndSpace();

	ImGui::Text("Controls Help:");
	ImGui::BulletText("Right Click + Drag: Rotate Camera");
	ImGui::BulletText("Right Click + Shift + Drag: Pan Camera");
	ImGui::BulletText("Mouse Scroll: Zoom");

	ImGui::End();
}
