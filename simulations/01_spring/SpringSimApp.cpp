#include "SpringSimApp.hpp"
#include "SpringSimUI.hpp"
#include "core/MeshGenerator.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
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
        glm::vec3(lightPos[0], lightPos[1], lightPos[2]),
        camera.getPosition()
    );

    // 1. Render Ceiling / Mount Plate at top (displacement w(t))
    float topY = springGen.getTopY(physics.w);
    glm::mat4 modelMount = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, topY, 0.0f));
    glm::vec4 mountColor(0.35f, 0.38f, 0.42f, 1.0f);
    renderer->drawMeshPhong(springGen.mountMesh, modelMount, mountColor);

    // 2. Render 3D Helical Spring
    if (springGen.springMesh) {
        glm::vec4 sColor(0.85f, 0.85f, 0.90f, 1.0f);
        if (springUI) {
            sColor = glm::vec4(springUI->springColor[0], springUI->springColor[1], springUI->springColor[2], 1.0f);
        }
        renderer->drawMeshPhong(springGen.springMesh, glm::mat4(1.0f), sColor);
    }

    // 3. Render Attached Cube at bottom end (displacement x(t))
    float cubeCenterY = springGen.getCubeCenterY(physics.x, cubeSize);
    glm::mat4 modelCube = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, cubeCenterY, 0.0f)) *
                          glm::scale(glm::mat4(1.0f), glm::vec3(cubeSize, cubeSize, cubeSize));
    glm::vec4 cColor(cubeColor[0], cubeColor[1], cubeColor[2], 1.0f);
    renderer->drawMeshPhong(cubeMesh, modelCube, cColor);

    // 4. Render Reference Guide Ticks / Ruler
    if (springGen.rulerMesh) {
        glm::vec4 rulerColor(0.45f, 0.50f, 0.55f, 0.8f);
        renderer->drawMeshPhong(springGen.rulerMesh, glm::mat4(1.0f), rulerColor);
    }
}
