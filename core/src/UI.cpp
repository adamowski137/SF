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

void UI::renderUI(Window& window)
{
	// Default base implementation: empty or simple info
	const ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse;

	ImGui::SetNextWindowPos(ImVec2(static_cast<float>(window.getViewportWidth()), 0.0f));
	ImGui::SetNextWindowSize(ImVec2(static_cast<float>(window.getWidth() - window.getViewportWidth()), static_cast<float>(window.getHeight())));

	ImGui::Begin("Info", nullptr, windowFlags);
	ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
	ImGui::End();
}

void UI::renderSepAndSpace()
{
	ImGui::Dummy(ImVec2(0.f, 10.f));
	ImGui::Separator();
	ImGui::Dummy(ImVec2(0.f, 10.f));
}
