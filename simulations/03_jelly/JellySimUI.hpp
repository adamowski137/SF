#pragma once
#include "core/UI.hpp"
#include "JellyPhysics.hpp"
#include <glm/glm.hpp>

struct JellyVisualSettings {
    bool showPoints = true;
    bool showShortSprings = true;
    bool showLongSprings = false;
    bool showFrame = true;
    bool showBox = true;
    bool showJelly = true;
    bool showObject = true;

    glm::vec4 pointColor{ 0.95f, 0.25f, 0.25f, 1.0f };       // red masses
    glm::vec4 shortSpringColor{ 0.25f, 0.25f, 0.30f, 1.0f }; // dark slate (contrast on cream background)
    glm::vec4 longSpringColor{ 0.50f, 0.50f, 0.58f, 1.0f };  // mid grey
    glm::vec4 frameColor{ 0.00f, 0.55f, 0.75f, 1.0f };       // teal
    glm::vec4 boxColor{ 0.55f, 0.50f, 0.42f, 1.0f };         // warm grey wireframe
    glm::vec4 boxWallColor{ 1.00f, 0.96f, 0.86f, 0.35f };    // transparent cream walls
    glm::vec4 jellyColor{ 0.35f, 0.85f, 0.35f, 0.55f };      // translucent green
    glm::vec4 objectColor{ 0.95f, 0.55f, 0.15f, 1.0f };      // warm amber

    glm::vec3 lightPos{ 5.0f, 10.0f, 7.0f };
    float pointRadius = 0.03f;
};

class JellySimUI : public UI {
public:
    JellyPhysics& physics;
    JellyVisualSettings visual;

public:
    JellySimUI(int width, JellyPhysics& phys);
    ~JellySimUI() override = default;

    void renderUI(Window& window) override;

private:
    void renderSimulationControls();
    void renderVisualizationTab();
    void renderPhysicsParametersTab();
};
