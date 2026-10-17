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
    PlotWidget ex4Plot;
    PlotWidget resonanceAmpPlot;
    PlotWidget resonancePhasePlot;
    PlotWidget settlingPlot;

    // Exercise test states
    std::vector<ConvergenceResult> ex1Results;
    bool hasEx1Results = false;

    float ex2ExpK = 0.0f;
    float ex2TheoryK = 0.0f;
    bool hasEx2Results = false;

    std::vector<SettlingPoint> ex3Results;
    float ex3OptimalK = 0.0f;
    float ex3MinSettlingT = 0.0f;
    bool hasEx3Results = false;

    std::vector<ResonanceCurve> ex5Curves;
    bool hasEx5Results = false;

public:
    SpringSimUI(int width, SpringPhysics& phys, float& inCubeSize, float* inCubeColor, float* inLightPos);
    ~SpringSimUI() override = default;

    void updatePlotData();
    void renderUI(Window& window) override;

private:
    void renderControlsPanel();
    void renderGraphsPanel();
    void renderExercisesPanel();
};
