#pragma once

#include "core/App.hpp"
#include "core/Mesh.hpp"
#include "CubePhysics.hpp"
#include "CubeSimUI.hpp"
#include <memory>

class CubeSimApp : public App {
private:
    CubePhysics physics;
    CubeSimUI* cubeUI = nullptr; // non-owning view of App::ui

    // Meshes
    std::shared_ptr<Mesh> cornerCubeMesh;
    std::shared_ptr<Mesh> cornerWireframeMesh;
    std::shared_ptr<Mesh> gridMesh;
    std::shared_ptr<Mesh> cylinderMesh;
    std::shared_ptr<Mesh> sphereMesh;
    std::shared_ptr<Mesh> trajectoryMesh;

    double lastFrameTime = 0.0;

public:
    CubeSimApp(int width, int height, int ui_width = 580);
    ~CubeSimApp() override = default;

protected:
    void init() override;
    void update(float dt) override;
    void renderScene() override;
    void processInput() override;
};
