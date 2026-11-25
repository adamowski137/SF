#pragma once

#include "core/App.hpp"
#include "core/Mesh.hpp"
#include "JellyPhysics.hpp"
#include "JellyMeshGenerator.hpp"
#include "JellySimUI.hpp"
#include <memory>
#include <vector>

class JellySimApp : public App {
private:
    JellyPhysics physics;
    JellySimUI* jellyUI = nullptr; // non-owning view of App::ui

    // Rest geometry in C = [0,1]^3, deformed every frame
    JellyMeshGenerator::MeshData jellyData, objectData;

    // Meshes
    std::shared_ptr<Mesh> jellyMesh, objectMesh, sphereMesh;
    std::shared_ptr<Mesh> boxMesh, boxWallsMesh, frameMesh, edgeSpringMesh, diagSpringMesh;
    std::vector<unsigned> frameIdx, edgeSpringIdx, diagSpringIdx;

    double lastFrameTime = 0.0;
    const float frameMoveSpeed = 0.0013f; // world units per pixel (scaled by camera distance)
    const float frameRotSpeed = 0.01f;    // radians per pixel

public:
    JellySimApp(int width, int height, int ui_width = 580);
    ~JellySimApp() override = default;

protected:
    void init() override;
    void update(float dt) override;
    void renderScene() override;
    void processInput() override;

private:
    void updateMeshes();
};
