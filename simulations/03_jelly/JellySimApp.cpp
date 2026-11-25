#include "JellySimApp.hpp"
#include "core/MeshGenerator.hpp"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

JellySimApp::JellySimApp(int width, int height, int ui_width)
    : App(width, height, ui_width, "SF - 03 Jelly (Dynamic Deformations) Simulation")
{
    camera.reset(-3.14159f / 4.0f, -3.14159f / 6.0f, 7.0f, glm::vec3(0.0f));

    auto customUI = std::make_unique<JellySimUI>(ui_width, physics);
    jellyUI = customUI.get();
    ui = std::move(customUI);
    ui->initImGui(window.getNativeWindow());
}

void JellySimApp::init() {
    // Sync with the real framebuffer size (the OS may shrink the window at creation, without a resize callback)
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window.getNativeWindow(), &fbWidth, &fbHeight);
    window.handleWindowResize(fbWidth, fbHeight);

    glClearColor(0.96f, 0.94f, 0.88f, 1.0f); // light cream background

    // Blending for translucent surfaces (ImGui backend restores this state after its own rendering)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    jellyData = JellyMeshGenerator::GetUnitCubeSurface(16);
    objectData = JellyMeshGenerator::GetTorusInUnitCube(0.3f, 0.65f, 24, 48);
    jellyMesh = Mesh::createMesh(jellyData.vertices, jellyData.indices, { {3, GL_FLOAT}, {3, GL_FLOAT} });
    objectMesh = Mesh::createMesh(objectData.vertices, objectData.indices, { {3, GL_FLOAT}, {3, GL_FLOAT} });
    sphereMesh = MeshGenerator::GetSphereMesh();

    // Static bounding box wireframe
    std::array<glm::vec3, 8> box;
    for (int n = 0; n < 8; ++n)
        box[n] = physics.boxHalf * glm::vec3((n >> 2) & 1 ? 1 : -1, (n >> 1) & 1 ? 1 : -1, n & 1 ? 1 : -1);
    boxMesh = Mesh::createMesh(JellyMeshGenerator::Flatten(box.data(), box.size()), JellyMeshGenerator::GetCubeEdgeIndices());
    auto walls = JellyMeshGenerator::GetUnitCubeSurface(1);
    boxWallsMesh = Mesh::createMesh(walls.vertices, walls.indices, { {3, GL_FLOAT}, {3, GL_FLOAT} });

    // Frame: 8 frame corners (0..7) + 8 cube corners (8..15); 12 frame edges + 8 corner connections
    frameIdx = JellyMeshGenerator::GetCubeEdgeIndices();
    for (unsigned n = 0; n < 8; ++n)
        frameIdx.insert(frameIdx.end(), { n, 8 + n });

    for (int s = 0; s < static_cast<int>(physics.springs.size()); ++s) {
        auto& idx = s < physics.edgeSpringCount ? edgeSpringIdx : diagSpringIdx;
        idx.insert(idx.end(), { (unsigned)physics.springs[s].i, (unsigned)physics.springs[s].j });
    }

    auto pts = JellyMeshGenerator::Flatten(physics.p.data(), physics.p.size());
    edgeSpringMesh = Mesh::createMesh(pts, edgeSpringIdx);
    diagSpringMesh = Mesh::createMesh(pts, diagSpringIdx);
    frameMesh = Mesh::createMesh(std::vector<float>(16 * 3, 0.0f), frameIdx);

    updateMeshes();
    lastFrameTime = glfwGetTime();
}

void JellySimApp::update(float dt) {
    (void)dt;
    double curTime = glfwGetTime();
    float realDt = static_cast<float>(curTime - lastFrameTime);
    lastFrameTime = curTime;

    if (realDt < 0.0f) realDt = 0.0f;
    if (realDt > 0.1f) realDt = 0.1f;

    physics.update(realDt);
    updateMeshes();
}

void JellySimApp::updateMeshes() {
    const auto& vis = jellyUI->visual;

    auto pts = JellyMeshGenerator::Flatten(physics.p.data(), physics.p.size());
    edgeSpringMesh->updateMesh(pts, edgeSpringIdx);
    diagSpringMesh->updateMesh(pts, diagSpringIdx);

    std::array<glm::vec3, 16> frameVerts;
    auto corners = physics.frameCorners();
    for (int n = 0; n < 8; ++n) {
        frameVerts[n] = corners[n];
        frameVerts[8 + n] = physics.p[JellyPhysics::cornerIndex(n)];
    }
    frameMesh->updateMesh(JellyMeshGenerator::Flatten(frameVerts.data(), frameVerts.size()), frameIdx);

    // Bezier deformation is done on the CPU, only for visible surfaces
    if (vis.showJelly)
        jellyMesh->updateMesh(JellyMeshGenerator::Deform(physics.p, jellyData.vertices), jellyData.indices);
    if (vis.showObject)
        objectMesh->updateMesh(JellyMeshGenerator::Deform(physics.p, objectData.vertices), objectData.indices);
}

void JellySimApp::processInput() {
    App::processInput(); // scene: RMB rotate, Shift+RMB pan, scroll zoom

    if (inputHandler.isPressed(ImGuiKey_Space, false))
        physics.isRunning = !physics.isRunning;

    // Control frame: LMB drag moves it in the view plane, Ctrl+LMB drag rotates it
    if (inputHandler.isPressed(ImGuiKey_MouseLeft, true)) {
        auto [dx, dy] = inputHandler.getMouseMove();
        glm::vec3 right = camera.getRightVector();
        glm::vec3 up = camera.getUpVector();

        if (inputHandler.isPressed(ImGuiKey_LeftCtrl, true)) {
            glm::quat r = glm::angleAxis(dx * frameRotSpeed, up) * glm::angleAxis(dy * frameRotSpeed, right);
            physics.frameRot = glm::normalize(r * physics.frameRot);
        } else {
            physics.framePos += (right * dx - up * dy) * frameMoveSpeed * camera.getRadius();
        }
    }
}

void JellySimApp::renderScene() {
    if (!renderer || !jellyUI) return;
    const auto& vis = jellyUI->visual;
    const glm::mat4 I(1.0f);

    renderer->setLighting(vis.lightPos, camera.getPosition());

    if (vis.showBox) {
        // Transparent box: only the far walls are shaded. The negative scale mirrors the unit cube onto the box,
        // turning normals inward and reversing winding, so back-face culling keeps exactly the far walls.
        glm::mat4 walls = glm::scale(I, -2.0f * physics.boxHalf) * glm::translate(I, glm::vec3(-0.5f));
        glDepthMask(GL_FALSE); // walls never hide anything drawn after them
        renderer->drawMeshPhong(boxWallsMesh, walls, vis.boxWallColor);
        glDepthMask(GL_TRUE);
        renderer->drawLines(boxMesh, I, vis.boxColor, 1.5f);
    }
    if (vis.showFrame)
        renderer->drawLines(frameMesh, I, vis.frameColor, 2.0f);
    if (vis.showShortSprings)
        renderer->drawLines(edgeSpringMesh, I, vis.shortSpringColor, 1.5f);
    if (vis.showLongSprings)
        renderer->drawLines(diagSpringMesh, I, vis.longSpringColor, 1.0f);

    if (vis.showPoints)
        for (const auto& p : physics.p)
            renderer->drawMeshPhong(sphereMesh, glm::translate(I, p) * glm::scale(I, glm::vec3(vis.pointRadius)), vis.pointColor);

    if (vis.showObject)
        renderer->drawMeshPhong(objectMesh, I, vis.objectColor);

    // Drawn last - translucent
    if (vis.showJelly)
        renderer->drawMeshPhong(jellyMesh, I, vis.jellyColor);
}
