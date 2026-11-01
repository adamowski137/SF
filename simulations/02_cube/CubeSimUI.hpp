#pragma once
#include "core/UI.hpp"
#include "CubePhysics.hpp"
#include "PlotWidget.hpp"
#include <glm/glm.hpp>
#include <memory>

class Camera;

struct CubeVisualSettings {
    bool showCube = true;
    bool showWireframe = true;
    bool showDiagonal = true;
    bool showTrajectory = true;
    bool showGravityVector = true;
    bool showPerpendicularPlane = true;
    bool showMarkers = true;
    bool showGrid = true;

    glm::vec4 cubeColor{ 0.95f, 0.55f, 0.15f, 0.90f };      // warm amber
    glm::vec4 wireframeColor{ 0.12f, 0.12f, 0.14f, 1.0f }; // sharp dark outline
    glm::vec4 diagonalColor{ 1.00f, 0.88f, 0.05f, 1.0f };   // bright gold
    glm::vec4 trajectoryColor{ 0.00f, 0.85f, 1.00f, 1.0f }; // cyan
    glm::vec4 gravityColor{ 0.20f, 0.90f, 0.35f, 1.0f };    // green
    glm::vec4 planeColor{ 0.30f, 0.55f, 0.85f, 0.35f };     // semi-transparent ice-blue
    glm::vec4 pivotColor{ 0.95f, 0.20f, 0.20f, 1.0f };     // red pivot anchor
    glm::vec4 tipColor{ 1.00f, 0.85f, 0.10f, 1.0f };       // gold tip marker

    glm::vec3 lightPos{ 5.0f, 10.0f, 7.0f };
    float diagonalRadius = 0.022f;
    float markerRadius = 0.055f;
    float gravityArrowLength = 1.8f;
    float planeSize = 8.0f;
};

class CubeSimUI : public UI {
public:
    CubePhysics& physics;
    CubeVisualSettings visual;
    Camera& camera;

    // Plots
    PlotWidget trajectoryPlot; // Horizontal plane X vs Z
    PlotWidget energyPlot;     // E_kin, E_pot, E_total
    PlotWidget angularVelPlot; // W_x, W_y, W_z
    PlotWidget inclinationPlot;// Theta(t)

    int activePresetIdx = 0;

public:
    CubeSimUI(int width, CubePhysics& phys, Camera& cam);
    ~CubeSimUI() override = default;

    void updatePlotData();
    void renderUI(Window& window) override;

private:
    void renderSimulationControls();
    void renderVisualizationTab();
    void renderPhysicsParametersTab();
    void renderTelemetryAndPlotsTab();
    void renderHelpSection();
};
