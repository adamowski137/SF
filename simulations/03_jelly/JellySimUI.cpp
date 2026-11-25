#include "JellySimUI.hpp"
#include "core/Window.hpp"
#include <glm/gtc/quaternion.hpp>

JellySimUI::JellySimUI(int width, JellyPhysics& phys)
    : UI(width), physics(phys)
{
}

void JellySimUI::renderUI(Window& window) {
    const ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse;

    // Anchor the panel to the right edge of the actual display (pivot = panel's top-right corner)
    const ImVec2 display = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos(ImVec2(display.x, 0.0f), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
    ImGui::SetNextWindowSize(ImVec2(static_cast<float>(window.getUiWidth()), display.y));

    ImGui::Begin("JellySimControlPanel", nullptr, windowFlags);

    ImGui::TextColored(ImVec4(1.0f, 0.82f, 0.2f, 1.0f), "LAB 3: DYNAMICZNE DEFORMACJE (ZELEK)");
    ImGui::TextDisabled("64 masy + ramka sterujaca + kostka Beziera");
    ImGui::Separator();

    renderSimulationControls();

    if (ImGui::BeginTabBar("JellySimTabs")) {
        if (ImGui::BeginTabItem("Wizualizacja")) {
            renderVisualizationTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Parametry i Fizyka")) {
            renderPhysicsParametersTab();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}

void JellySimUI::renderSimulationControls() {
    ImGui::Spacing();

    // Play / Pause / Step / Reset / Disturb buttons
    if (physics.isRunning) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.2f, 0.2f, 1.0f));
        if (ImGui::Button("Pauza [Space]", ImVec2(110, 28))) {
            physics.isRunning = false;
        }
        ImGui::PopStyleColor();
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.3f, 1.0f));
        if (ImGui::Button("Start [Space]", ImVec2(110, 28))) {
            physics.isRunning = true;
        }
        ImGui::PopStyleColor();
    }

    ImGui::SameLine();
    if (ImGui::Button("Krok", ImVec2(60, 28))) {
        physics.isRunning = false;
        physics.step();
    }

    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.4f, 0.6f, 1.0f));
    if (ImGui::Button("Reset", ImVec2(65, 28))) {
        physics.reset();
    }
    ImGui::PopStyleColor();

    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.5f, 0.1f, 1.0f));
    if (ImGui::Button("Zaburz", ImVec2(70, 28))) {
        physics.disturb();
    }
    ImGui::PopStyleColor();

    ImGui::SameLine();
    ImGui::Text("t: %.2f s | FPS: %.0f", physics.t, ImGui::GetIO().Framerate);

    // Speed multiplier and physics dt
    ImGui::SliderFloat("Predkosc symulacji", &physics.simTimeSpeed, 0.1f, 4.0f, "%.1fx");
    ImGui::SliderFloat("Krok calkowania dt", &physics.delta, 0.0001f, 0.005f, "%.4f s");

    ImGui::Separator();
}

void JellySimUI::renderVisualizationTab() {
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.4f, 0.85f, 1.0f, 1.0f), "1. WYSWIETLANIE NA ZYCZENIE (Krok 1.1)");

    ImGui::Checkbox("Punkty kontrolne (masy)", &visual.showPoints);
    ImGui::Checkbox("Sprezyny krotkie", &visual.showShortSprings);
    ImGui::SameLine(230);
    ImGui::Checkbox("Sprezyny przekatne", &visual.showLongSprings);
    ImGui::Checkbox("Ramka sterujaca i polaczenia", &visual.showFrame);
    ImGui::Checkbox("Prostopadloscian ograniczajacy", &visual.showBox);
    ImGui::Checkbox("Kostka Beziera (zelek)", &visual.showJelly);
    ImGui::Checkbox("Zdeformowana bryla", &visual.showObject);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.4f, 0.85f, 1.0f, 1.0f), "2. INTERAKCJA ZE SCENA (Krok 1.2)");
    ImGui::BulletText("PPM: obrot sceny, Shift+PPM: przesuwanie, kolko: skalowanie");
    ImGui::BulletText("LPM: przesuwanie ramki, Ctrl+LPM: obrot ramki");
}

void JellySimUI::renderPhysicsParametersTab() {
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.4f, 0.85f, 1.0f, 1.0f), "PARAMETRY UKLADU (Krok 1.3)");

    ImGui::SliderFloat("Masa punktu m", &physics.m, 0.01f, 100.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
    ImGui::SliderFloat("Sprezystosc c1", &physics.c1, 0.01f, 100.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
    ImGui::SliderFloat("Tlumienie k", &physics.k, 0.0f, 100.0f, "%.2f");
    ImGui::SliderFloat("Sprezystosc ramki c2", &physics.c2, 0.0f, 100.0f, "%.2f");
    ImGui::SliderFloat("Maks. zaburzenie |v|", &physics.disturbance, 0.0f, 20.0f, "%.2f");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.4f, 0.85f, 1.0f, 1.0f), "KOLIZJE (Krok 4)");
    ImGui::SliderFloat("Wsp. odbicia mu", &physics.mu, 0.0f, 1.0f, "%.2f");
    ImGui::TextDisabled("Model: v po odbiciu = (-mu*vx, vy, vz)");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.4f, 0.85f, 1.0f, 1.0f), "RAMKA STERUJACA (Krok 3)");
    ImGui::DragFloat3("Pozycja ramki", &physics.framePos.x, 0.01f);

    glm::vec3 euler = glm::degrees(glm::eulerAngles(physics.frameRot));
    if (ImGui::DragFloat3("Obrot ramki [deg]", &euler.x, 0.5f)) {
        physics.frameRot = glm::quat(glm::radians(euler));
    }

    if (ImGui::Button("Wyzeruj ramke")) {
        physics.framePos = glm::vec3(0.0f);
        physics.frameRot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    }
}
