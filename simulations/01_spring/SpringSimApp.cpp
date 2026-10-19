#include "SpringSimApp.hpp"
#include "core/MeshGenerator.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

SpringSimApp::SpringSimApp(int width, int height, int ui_width)
    : App(width, height, ui_width, "SF - 01 Helical Spring & Mass Simulation")
{
    ui = std::make_unique<SpringSimUI>(ui_width, physics, visual);
    ui->initImGui(window.getNativeWindow());

    camera.zoom(-2.5f);
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
    renderer->setLighting(visual.lightPos, camera.getPosition());

    float topY = springGen.getTopY(physics.w);
    glm::mat4 modelMount = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, topY, 0.0f));
    glm::vec4 mountColor(0.35f, 0.38f, 0.42f, 1.0f);
    renderer->drawMeshPhong(springGen.mountMesh, modelMount, mountColor);

    if (springGen.springMesh) {
        renderer->drawMeshPhong(springGen.springMesh, glm::mat4(1.0f), glm::vec4(visual.springColor, 1.0f));
    }

    float cubeCenterY = springGen.getCubeCenterY(physics.x, visual.cubeSize);
    glm::mat4 modelCube = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, cubeCenterY, 0.0f)) *
                          glm::scale(glm::mat4(1.0f), glm::vec3(visual.cubeSize));
    renderer->drawMeshPhong(cubeMesh, modelCube, glm::vec4(visual.cubeColor, 1.0f));

    if (springGen.rulerMesh) {
        glm::vec4 rulerColor(0.45f, 0.50f, 0.55f, 0.8f);
        renderer->drawMeshPhong(springGen.rulerMesh, glm::mat4(1.0f), rulerColor);
    }
}
