#pragma once

#include "core/App.hpp"
#include "core/Mesh.hpp"
#include "SpringPhysics.hpp"
#include "SpringMeshGenerator.hpp"
#include <memory>

class SpringSimUI;

class SpringSimApp : public App {
    SpringPhysics physics;
    SpringMeshGenerator springGen;
    std::shared_ptr<Mesh> cubeMesh;
    SpringSimUI* springUI = nullptr;

    double lastFrameTime = 0.0;

    // Visual state
    float cubeSize = 0.85f;
    float cubeColor[3] = { 0.95f, 0.75f, 0.2f }; // Golden cube
    float lightPos[3] = { 4.0f, 8.0f, 6.0f };

public:
    SpringSimApp(int width, int height, int ui_width = 600);
    ~SpringSimApp() override = default;

protected:
    void init() override;
    void update(float dt) override;
    void renderScene() override;
};
