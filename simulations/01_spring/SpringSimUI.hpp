#pragma once
#include "core/UI.hpp"
#include "SpringPhysics.hpp"
#include "PlotWidget.hpp"
#include <glm/glm.hpp>
#include <string>

struct VisualSettings {
    float cubeSize = 0.85f;
    glm::vec3 cubeColor{ 0.95f, 0.75f, 0.2f };      // Golden cube
    glm::vec3 springColor{ 0.85f, 0.85f, 0.90f };   // Shiny metallic silver
    glm::vec3 lightPos{ 4.0f, 8.0f, 6.0f };
};

class SpringSimUI : public UI {
public:
    SpringPhysics& physics;
    VisualSettings& visual;

    // Plotting widgets
    PlotWidget forcesPlot;
    PlotWidget kinematicsPlot;
    PlotWidget trajectoryPlot;

public:
    SpringSimUI(int width, SpringPhysics& phys, VisualSettings& vis);
    ~SpringSimUI() override = default;

    void updatePlotData();
    void renderUI(Window& window) override;

private:
    void renderControlsPanel();
    void renderGraphsPanel();
    void renderSignalFunctionControls(
        const char* title, const char* prefix, const ImVec4& headerColor, SignalFunction& func, float maxAmp);
};
