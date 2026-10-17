#pragma once
#include "core/UI.hpp"
#include "SpringPhysics.hpp"
#include "PlotWidget.hpp"
#include <string>

class SpringSimUI : public UI {
public:
    SpringPhysics& physics;

    // Visual rendering properties
    float& cubeSize;
    float* cubeColor;
    float* lightPos;
    float springColor[3] = { 0.85f, 0.85f, 0.90f }; // Shiny metallic silver

    // Plotting widgets
    PlotWidget forcesPlot;
    PlotWidget kinematicsPlot;
    PlotWidget trajectoryPlot;
    PlotWidget errorPlot;

public:
    SpringSimUI(int width, SpringPhysics& phys, float& inCubeSize, float* inCubeColor, float* inLightPos);
    ~SpringSimUI() override = default;

    void updatePlotData();
    void renderUI(Window& window) override;

private:
    void renderControlsPanel();
    void renderGraphsPanel();
};
