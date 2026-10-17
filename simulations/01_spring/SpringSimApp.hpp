#pragma once

#include "core/App.hpp"
#include "core/Mesh.hpp"
#include "SpringPhysics.hpp"
#include "SpringMeshGenerator.hpp"
#include "SpringSimUI.hpp"
#include <memory>

class SpringSimApp : public App {
    SpringPhysics physics;
    SpringMeshGenerator springGen;
    std::shared_ptr<Mesh> cubeMesh;
    VisualSettings visual;

    double lastFrameTime = 0.0;

public:
    SpringSimApp(int width, int height, int ui_width = 600);
    ~SpringSimApp() override = default;

protected:
    void init() override;
    void update(float dt) override;
    void renderScene() override;
};
