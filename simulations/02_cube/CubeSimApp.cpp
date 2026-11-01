#include "CubeSimApp.hpp"
#include "CubeMeshGenerator.hpp"
#include "core/MeshGenerator.hpp"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

CubeSimApp::CubeSimApp(int width, int height, int ui_width)
    : App(width, height, ui_width, "SF - 02 Spinning Cube (Baczek) Simulation")
{
    // Setup camera position and orientation for great initial 3D perspective
    camera.reset(-3.14159f / 4.0f, -3.14159f / 6.0f, 4.5f, glm::vec3(0.0f, 0.5f, 0.0f));

    // Initialize UI
    auto customUI = std::make_unique<CubeSimUI>(ui_width, physics, camera);
    cubeUI = customUI.get();
    ui = std::move(customUI);
    ui->initImGui(window.getNativeWindow());
}

void CubeSimApp::init() {
    cornerCubeMesh = CubeMeshGenerator::GetCornerCubeMesh();
    cornerWireframeMesh = CubeMeshGenerator::GetCornerCubeWireframeMesh();
    planeMesh = CubeMeshGenerator::GetPlaneMesh(1.0f); // unit plane, scaled in draw
    gridMesh = CubeMeshGenerator::GetGridMesh(12.0f, 0.5f);
    cylinderMesh = MeshGenerator::GetCylinderMesh(1.0f, 1.0f, 24, 2);
    sphereMesh = MeshGenerator::GetSphereMesh();
    trajectoryMesh = CubeMeshGenerator::CreateTrajectoryMesh();

    lastFrameTime = glfwGetTime();
}

void CubeSimApp::update(float dt) {
    (void)dt;
    double curTime = glfwGetTime();
    float realDt = static_cast<float>(curTime - lastFrameTime);
    lastFrameTime = curTime;

    if (realDt < 0.0f) realDt = 0.0f;
    if (realDt > 0.1f) realDt = 0.1f;

    physics.update(realDt);

    // Update dynamic OpenGL trajectory line mesh
    if (physics.trajectory.size() >= 2) {
        CubeMeshGenerator::UpdateTrajectoryMesh(trajectoryMesh, physics.trajectory);
    }

    // Refresh UI plot data buffers
    if (cubeUI) {
        cubeUI->updatePlotData();
    }
}

void CubeSimApp::processInput() {
    App::processInput();

    // Toggle Play / Pause with Space
    if (inputHandler.isPressed(ImGuiKey_Space, false)) {
        physics.isRunning = !physics.isRunning;
    }
}

void CubeSimApp::renderScene() {
    if (!renderer || !cubeUI) return;

    renderer->setLighting(cubeUI->visual.lightPos, camera.getPosition());

    // 1. Reference floor grid
    if (cubeUI->visual.showGrid && gridMesh) {
        renderer->drawLines(gridMesh, glm::mat4(1.0f), glm::vec4(0.24f, 0.28f, 0.35f, 0.6f), 1.0f);
    }

    // 2. Semi-transparent plane perpendicular to gravity vector (Krok 1.1)
    if (cubeUI->visual.showPerpendicularPlane && planeMesh) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_CULL_FACE);

        // Plane normal should point opposite to gravity direction
        glm::vec3 upDir = -glm::normalize(physics.gravityDir);
        glm::quat rotPlane = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        if (glm::length(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), upDir)) > 1e-4f) {
            float cosT = glm::clamp(glm::dot(glm::vec3(0.0f, 1.0f, 0.0f), upDir), -1.0f, 1.0f);
            rotPlane = glm::angleAxis(std::acos(cosT), glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), upDir)));
        }

        glm::mat4 modelPlane = glm::mat4_cast(rotPlane) *
                               glm::scale(glm::mat4(1.0f), glm::vec3(cubeUI->visual.planeSize));
        renderer->drawMeshPhong(planeMesh, modelPlane, cubeUI->visual.planeColor);

        glEnable(GL_CULL_FACE);
        glDisable(GL_BLEND);
    }

    // 3. Gravity direction vector (arrow) (Krok 1.1)
    if (cubeUI->visual.showGravityVector && cylinderMesh) {
        glm::vec3 gDirNorm = glm::normalize(physics.gravityDir);
        float arrowLen = cubeUI->visual.gravityArrowLength;
        glm::vec3 startPos(0.0f, 0.0f, 0.0f);
        glm::vec3 endPos = startPos + gDirNorm * arrowLen;
        float shaftRadius = 0.015f;
        float headLen = 0.25f;
        float headRadius = 0.05f;

        // Shaft
        glm::vec3 shaftEnd = endPos - gDirNorm * headLen;
        glm::mat4 modelShaft = CubeMeshGenerator::ComputeCylinderTransform(startPos, shaftEnd, shaftRadius);
        renderer->drawMeshPhong(cylinderMesh, modelShaft, cubeUI->visual.gravityColor);

        // Arrowhead (Cone / Cylinder)
        static auto coneMesh = CubeMeshGenerator::GetConeMesh(headRadius, headLen, 24);
        glm::mat4 modelCone = CubeMeshGenerator::ComputeConeTransform(endPos, gDirNorm, headLen, 1.0f);
        renderer->drawMeshPhong(coneMesh, modelCone, cubeUI->visual.gravityColor);
    }

    // 4. Cube model matrix: anchored at fixed vertex (0, 0, 0)
    glm::mat4 modelCube = glm::mat4_cast(physics.Q) *
                          glm::scale(glm::mat4(1.0f), glm::vec3(physics.a, physics.b, physics.c));

    // Solid cube
    if (cubeUI->visual.showCube && cornerCubeMesh) {
        renderer->drawMeshPhong(cornerCubeMesh, modelCube, cubeUI->visual.cubeColor);
    }

    // Wireframe edges
    if (cubeUI->visual.showWireframe && cornerWireframeMesh) {
        renderer->drawLines(cornerWireframeMesh, modelCube, cubeUI->visual.wireframeColor, 2.5f);
    }

    // 5. Cube diagonal from fixed corner to free tip (Krok 1.1)
    glm::vec3 tipPos = physics.getTipWorldPosition();
    if (cubeUI->visual.showDiagonal && cylinderMesh) {
        glm::mat4 modelDiag = CubeMeshGenerator::ComputeCylinderTransform(glm::vec3(0.0f), tipPos, cubeUI->visual.diagonalRadius);
        renderer->drawMeshPhong(cylinderMesh, modelDiag, cubeUI->visual.diagonalColor);
    }

    // 6. Characteristic points (fixed pivot anchor & free diagonal tip)
    if (cubeUI->visual.showMarkers && sphereMesh) {
        // Pivot anchor at origin (0, 0, 0)
        glm::mat4 modelPivot = glm::scale(glm::mat4(1.0f), glm::vec3(cubeUI->visual.markerRadius));
        renderer->drawMeshPhong(sphereMesh, modelPivot, cubeUI->visual.pivotColor);

        // Free tip
        glm::mat4 modelTip = glm::translate(glm::mat4(1.0f), tipPos) *
                             glm::scale(glm::mat4(1.0f), glm::vec3(cubeUI->visual.markerRadius));
        renderer->drawMeshPhong(sphereMesh, modelTip, cubeUI->visual.tipColor);
    }

    // 7. Trajectory of the free diagonal end (Krok 1.1 & 1.3)
    if (cubeUI->visual.showTrajectory && trajectoryMesh && physics.trajectory.size() >= 2) {
        renderer->drawLines(trajectoryMesh, glm::mat4(1.0f), cubeUI->visual.trajectoryColor, 2.5f);
    }
}
