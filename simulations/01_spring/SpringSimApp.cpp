#include "SpringSimApp.hpp"
#include "SpringSimUI.hpp"
#include "core/MeshGenerator.hpp"
#include <matrix/matrix.hpp>
#include <GLFW/glfw3.h>

SpringSimApp::SpringSimApp(int width, int height, int ui_width)
    : App(width, height, ui_width, "SF - 01 Helical Spring & Mass Simulation")
{
    auto simUI = std::make_unique<SpringSimUI>(ui_width, physics, cubeSize, cubeColor, lightPos);
    springUI = simUI.get();
    ui = std::move(simUI);
    ui->initImGui(window.getNativeWindow());

    camera.zoom(-2.5f); // Step back to comfortably frame the entire spring and mass
}

void SpringSimApp::init() {
    cubeMesh = MeshGenerator::GetCubeMesh();
    springGen.init();
    lastFrameTime = glfwGetTime();
}

void SpringSimApp::update(float dt) {
    (void)dt;
    double curTime = glfwGetTime();
    float realDt = static_cast<float>(curTime - lastFrameTime);
    lastFrameTime = curTime;

    if (realDt <= 0.0f || realDt > 0.1f) {
        realDt = 0.016f;
    }

    physics.update(realDt);
    springGen.updateSpring(physics.w, physics.x);
}

void SpringSimApp::renderScene() {
    renderer->setLighting(
        Vector4<float>(lightPos[0], lightPos[1], lightPos[2], 1.0f),
        camera.getPosition()
    );

    // 1. Render Ceiling / Mount Plate at top (displacement w(t))
    float topY = springGen.getTopY(physics.w);
    Matrix4x4<float> modelMount = Matrix4x4<float>::translate(0.0f, topY, 0.0f);
    Vector4<float> mountColor(0.35f, 0.38f, 0.42f, 1.0f);
    renderer->drawMeshPhong(springGen.mountMesh, modelMount, mountColor);

    // 2. Render 3D Helical Spring
    if (springGen.springMesh) {
        Vector4<float> sColor(0.85f, 0.85f, 0.90f, 1.0f);
        if (springUI) {
            sColor = Vector4<float>(springUI->springColor[0], springUI->springColor[1], springUI->springColor[2], 1.0f);
        }
        renderer->drawMeshPhong(springGen.springMesh, Matrix4x4<float>::identity(), sColor);
    }

    // 3. Render Attached Cube at bottom end (displacement x(t))
    float cubeCenterY = springGen.getCubeCenterY(physics.x, cubeSize);
    Matrix4x4<float> modelCube = Matrix4x4<float>::translate(0.0f, cubeCenterY, 0.0f) *
                                 Matrix4x4<float>::scale(cubeSize, cubeSize, cubeSize);
    Vector4<float> cColor(cubeColor[0], cubeColor[1], cubeColor[2], 1.0f);
    renderer->drawMeshPhong(cubeMesh, modelCube, cColor);

    // 4. Render Reference Guide Ticks / Ruler
    if (springGen.rulerMesh) {
        Vector4<float> rulerColor(0.45f, 0.50f, 0.55f, 0.8f);
        renderer->drawMeshPhong(springGen.rulerMesh, Matrix4x4<float>::identity(), rulerColor);
    }
}
