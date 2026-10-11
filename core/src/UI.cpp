#include "core/UI.hpp"
#include "core/Window.hpp"

UI::UI(int width) : imGuiInitialized(false), width(width)
{
}

UI::~UI()
{
	if (imGuiInitialized)
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}
}

void UI::initImGui(GLFWwindow* window)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 430");

	imGuiInitialized = true;
}

void UI::startImGuiFrame()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void UI::endImGuiFrame()
{
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UI::renderSimpleSceneUI(Window& window, float& cubeSize, float cubeColor[3], float lightPos[3])
{
	const ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse;

	ImGui::SetNextWindowPos(ImVec2(static_cast<float>(window.getViewportWidth()), 0.0f));
	ImGui::SetNextWindowSize(ImVec2(static_cast<float>(window.getWidth() - window.getViewportWidth()), static_cast<float>(window.getHeight())));

	ImGui::Begin("Controls", nullptr, windowFlags);
	ImGui::Dummy(ImVec2(0.f, 10.f));

	ImGui::Text("SF Engine - Core Foundation");
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

void UI::renderSepAndSpace()
{
	ImGui::Dummy(ImVec2(0.f, 10.f));
	ImGui::Separator();
	ImGui::Dummy(ImVec2(0.f, 10.f));
}
