#include "SpringSimUI.hpp"
#include "core/Window.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <cstdio>

SpringSimUI::SpringSimUI(int width, SpringPhysics& phys, VisualSettings& vis)
    : UI(width), physics(phys), visual(vis),
      forcesPlot("Forces Plot"),
      kinematicsPlot("Kinematics Plot"),
    positionExtensionPlot("Position and Spring Extension"),
      trajectoryPlot("Phase Space Trajectory", true),
      errorPlot("Simulation vs Exact Solution")
{
    forcesPlot.addSeries("f(t) Spring force", IM_COL32(0, 210, 255, 255), 2.0f);
    forcesPlot.addSeries("g(t) Damping force", IM_COL32(60, 220, 100, 255), 2.0f);
    forcesPlot.addSeries("h(t) Outer force", IM_COL32(255, 170, 40, 255), 2.0f);
    forcesPlot.addSeries("w(t) Free end", IM_COL32(230, 90, 230, 255), 2.0f);

    kinematicsPlot.addSeries("x(t) Position", IM_COL32(245, 245, 255, 255), 2.0f);
    kinematicsPlot.addSeries("xt(t) Velocity", IM_COL32(50, 230, 120, 255), 2.0f);
    kinematicsPlot.addSeries("xtt(t) Acceleration", IM_COL32(60, 180, 255, 255), 2.0f);
    positionExtensionPlot.addSeries("x(t) Position", IM_COL32(245, 245, 255, 255), 2.0f);
    positionExtensionPlot.addSeries("w(t) Free end", IM_COL32(230, 90, 230, 255), 2.0f);
    positionExtensionPlot.addSeries("w(t) - x(t) Spring extension", IM_COL32(255, 170, 40, 255), 2.0f);

    trajectoryPlot.addSeries("Phase curve (x, xt)", IM_COL32(255, 75, 75, 255), 2.0f);

    // Initialize error plot series
    errorPlot.addSeries("xs(t) Simulated", IM_COL32(0, 210, 255, 255), 2.0f);
    errorPlot.addSeries("xe(t) Exact analytical", IM_COL32(255, 220, 50, 255), 2.0f);
    errorPlot.addSeries("xs(t) - xe(t) Error", IM_COL32(255, 75, 75, 255), 2.0f);
}

void SpringSimUI::updatePlotData() {
    const auto& hist = physics.history;
    if (hist.empty()) return;

    auto populate = [&](PlotWidget& plot, size_t idx, auto extractor) {
        auto& pts = plot.series[idx].points;
        pts.clear();
        pts.reserve(hist.size());
        for (const auto& p : hist) {
            pts.push_back(extractor(p));
        }
    };

    populate(forcesPlot, 0, [](const SimPoint& p) { return ImVec2(p.t, p.f); });
    populate(forcesPlot, 1, [](const SimPoint& p) { return ImVec2(p.t, p.g); });
    populate(forcesPlot, 2, [](const SimPoint& p) { return ImVec2(p.t, p.h); });
    populate(forcesPlot, 3, [](const SimPoint& p) { return ImVec2(p.t, p.w); });

    populate(kinematicsPlot, 0, [](const SimPoint& p) { return ImVec2(p.t, p.x); });
    populate(kinematicsPlot, 1, [](const SimPoint& p) { return ImVec2(p.t, p.v); });
    populate(kinematicsPlot, 2, [](const SimPoint& p) { return ImVec2(p.t, p.a); });

    populate(positionExtensionPlot, 0, [](const SimPoint& p) { return ImVec2(p.t, p.x); });
    populate(positionExtensionPlot, 1, [](const SimPoint& p) { return ImVec2(p.t, p.w); });
    populate(positionExtensionPlot, 2, [](const SimPoint& p) { return ImVec2(p.t, p.w - p.x); });

    populate(trajectoryPlot, 0, [](const SimPoint& p) { return ImVec2(p.x, p.v); });

    populate(errorPlot, 0, [](const SimPoint& p) { return ImVec2(p.t, p.x); });
    populate(errorPlot, 1, [](const SimPoint& p) { return ImVec2(p.t, p.xe); });
    populate(errorPlot, 2, [](const SimPoint& p) { return ImVec2(p.t, p.err); });
}

void SpringSimUI::renderUI(Window& window) {
    const ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse;

    float uiPanelX = static_cast<float>(window.getViewportWidth());
    float uiPanelWidth = static_cast<float>(window.getWidth() - window.getViewportWidth());
    float uiPanelHeight = static_cast<float>(window.getHeight());

    ImGui::SetNextWindowPos(ImVec2(uiPanelX, 0.0f));
    ImGui::SetNextWindowSize(ImVec2(uiPanelWidth, uiPanelHeight));

    ImGui::Begin("Spring Controls Panel", nullptr, windowFlags);

    ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "Helical Spring & Mass Simulation (Mini/PW)");
    ImGui::Text("FPS: %.1f | Sim Time: %.2f s", ImGui::GetIO().Framerate, physics.t);

    ImGui::Separator();

    if (ImGui::BeginTabBar("MainSimulationTabs")) {
        if (ImGui::BeginTabItem("Controls & State")) {
            renderControlsPanel();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Live Graphs")) {
            updatePlotData();
            renderGraphsPanel();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

void SpringSimUI::renderSignalFunctionControls(
    const char* title, const char* prefix, const ImVec4& headerColor, SignalFunction& func, float maxAmp)
{
    ImGui::TextColored(headerColor, "%s", title);
    const char* funcNames[] = {
        "Constant (f1 = A)",
        "Step (f2: 0 for t<0, A for t>=0)",
        "Square wave (f3 = sgn(A*sin(wt+phi)))",
        "Sinusoidal (f4 = A*sin(wt+phi))",
        "Exercise 6: w(t)"
    };
    int typeIdx = static_cast<int>(func.type);
    char comboLabel[64];
    std::snprintf(comboLabel, sizeof(comboLabel), "%s Type", prefix);
    if (ImGui::Combo(comboLabel, &typeIdx, funcNames, 5)) {
        func.type = static_cast<FunctionType>(typeIdx);
    }

    char ampLabel[64];
    std::snprintf(ampLabel, sizeof(ampLabel), "%s Amplitude A", prefix);
    ImGui::SliderFloat(ampLabel, &func.amplitude, -maxAmp, maxAmp);

    if (func.type == FunctionType::Square || func.type == FunctionType::Sine) {
        char freqLabel[64];
        char phiLabel[64];
        std::snprintf(freqLabel, sizeof(freqLabel), "%s Frequency w", prefix);
        std::snprintf(phiLabel, sizeof(phiLabel), "%s Phase phi", prefix);
        ImGui::SliderFloat(freqLabel, &func.omega, 0.1f, 15.0f);
        ImGui::SliderFloat(phiLabel, &func.phi, -3.1415f, 3.1415f);
    }
}

void SpringSimUI::renderControlsPanel() {
    ImGui::Spacing();

    if (physics.isRunning) {
        if (ImGui::Button("Pause", ImVec2(75, 28))) physics.isRunning = false;
    } else {
        if (ImGui::Button("Play", ImVec2(75, 28))) physics.isRunning = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Step", ImVec2(75, 28))) {
        physics.isRunning = false;
        physics.step();
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset", ImVec2(75, 28))) {
        physics.reset();
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(120.0f);
    ImGui::SliderFloat("Speed", &physics.simTimeSpeed, 0.1f, 4.0f, "%.1fx");

    renderSepAndSpace();

    ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.3f, 1.0f), "Live Simulation Telemetry");
    ImGui::Columns(2, "telemetry_cols", false);
    ImGui::Text("Position x:   %.3f", physics.x);
    ImGui::Text("Velocity xt:  %.3f", physics.v);
    ImGui::Text("Accel xtt:    %.3f", physics.a);
    ImGui::NextColumn();
    ImGui::Text("Spring f(t):  %.3f", physics.f);
    ImGui::Text("Damping g(t): %.3f", physics.g);
    ImGui::Text("Outer h(t):   %.3f", physics.h);
    ImGui::Text("Free end w(t):%.3f", physics.w);
    ImGui::Columns(1);

    renderSepAndSpace();

    ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), "Physical Parameters");

    bool paramChanged = false;
    paramChanged |= ImGui::SliderFloat("x(0) Initial Pos", &physics.x0, -10.0f, 10.0f, "%.2f");
    paramChanged |= ImGui::SliderFloat("v(0) Initial Vel", &physics.v0, -10.0f, 10.0f, "%.2f");
    paramChanged |= ImGui::SliderFloat("m Mass", &physics.m, 0.1f, 10.0f, "%.2f");
    paramChanged |= ImGui::SliderFloat("c Stiffness", &physics.c, 0.1f, 20.0f, "%.2f");
    paramChanged |= ImGui::SliderFloat("k Damping", &physics.k, 0.0f, 10.0f, "%.3f");
    ImGui::SliderFloat("Delta (Step dt)", &physics.delta, 0.001f, 0.1f, "%.3f");

    if (paramChanged) {
        physics.reset();
    }

    ImGui::TextDisabled("Integrator: Euler");

    renderSepAndSpace();

    renderSignalFunctionControls("Free End Function w(t)", "w", ImVec4(0.9f, 0.4f, 0.9f, 1.0f), physics.wFunc, 10.0f);

    if (physics.wFunc.type == FunctionType::Exercise6) {
        ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.6f, 1.0f),
            "Ex6: Time-optimal transfer 0->1 in T=2s");
        ImGui::TextDisabled(
            "Bang-bang: accel +1 for t<1, decel -1 for t<2, hold at x=1.\n"
            "w(t) = x*(t) + (k/c)*v*(t) + (m/c)*a*(t)\n"
            "Requires x(0)=0, v(0)=0, h(t)=0.");
        if (physics.x0 != 0.0f || physics.v0 != 0.0f) {
            physics.x0 = 0.0f;
            physics.v0 = 0.0f;
            physics.reset();
        }
    }

    renderSepAndSpace();

    renderSignalFunctionControls("Outer Field Force h(t)", "h", ImVec4(1.0f, 0.6f, 0.2f, 1.0f), physics.hFunc, 20.0f);

    renderSepAndSpace();

    if (ImGui::CollapsingHeader("3D Visual & Lighting Properties")) {
        ImGui::SliderFloat("Cube Size", &visual.cubeSize, 0.2f, 3.0f);
        ImGui::ColorEdit3("Cube Color", glm::value_ptr(visual.cubeColor));
        ImGui::ColorEdit3("Spring Color", glm::value_ptr(visual.springColor));
        ImGui::SliderFloat3("Light Position", glm::value_ptr(visual.lightPos), -20.0f, 20.0f);
    }

    ImGui::Spacing();
    ImGui::TextDisabled("Controls Help:\n- RMB + Drag: Rotate Camera\n- RMB + Shift + Drag: Pan Camera\n- Mouse Scroll: Zoom");
}

void SpringSimUI::renderGraphsPanel() {
    ImGui::Spacing();

    if (ImGui::BeginTabBar("GraphsSubTabs")) {
        if (ImGui::BeginTabItem("Forces [f, g, h, w]")) {
            ImGui::Text("Forces acting on mass m as function of time t:");
            forcesPlot.render(ImVec2(-1.0f, 240.0f));
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Kinematics [x, xt, xtt]")) {
            ImGui::Text("Position x(t), Velocity xt(t), and Acceleration xtt(t):");
            kinematicsPlot.render(ImVec2(-1.0f, 240.0f));
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Position & Spring Extension [x, w, w-x]")) {
            ImGui::Text("Position x(t), free end w(t), and spring extension w(t) - x(t):");
            positionExtensionPlot.render(ImVec2(-1.0f, 240.0f));
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Trajectory (Phase Space)")) {
            ImGui::Text("State trajectory (x(t), xt(t)) in phase space:");
            trajectoryPlot.render(ImVec2(-1.0f, 280.0f));
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Simulation vs Analytical Error")) {
            ImGui::Text("Difference xs(t) - xe(t) with exact analytical solution:");
            errorPlot.render(ImVec2(-1.0f, 240.0f));
            ImGui::Text("Current Error: %.6f | Max Absolute Error: %.6f",
                std::abs(physics.err), physics.maxAbsoluteError);
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
}
