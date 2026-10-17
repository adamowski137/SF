#include "SpringSimUI.hpp"
#include "core/Window.hpp"
#include <cstring>
#include <cmath>

SpringSimUI::SpringSimUI(int width, SpringPhysics& phys, float& inCubeSize, float* inCubeColor, float* inLightPos)
    : UI(width), physics(phys), cubeSize(inCubeSize), cubeColor(inCubeColor), lightPos(inLightPos),
      forcesPlot("Forces Plot"),
      kinematicsPlot("Kinematics Plot"),
      trajectoryPlot("Phase Space Trajectory", true),
      errorPlot("Simulation vs Exact Solution")
{
    // Initialize forces plot series
    forcesPlot.addSeries("f(t) Spring force", IM_COL32(0, 210, 255, 255), 2.0f);
    forcesPlot.addSeries("g(t) Damping force", IM_COL32(60, 220, 100, 255), 2.0f);
    forcesPlot.addSeries("h(t) Outer force", IM_COL32(255, 170, 40, 255), 2.0f);
    forcesPlot.addSeries("w(t) Free end", IM_COL32(230, 90, 230, 255), 2.0f);

    // Initialize kinematics plot series
    kinematicsPlot.addSeries("x(t) Position", IM_COL32(245, 245, 255, 255), 2.0f);
    kinematicsPlot.addSeries("xt(t) Velocity", IM_COL32(50, 230, 120, 255), 2.0f);
    kinematicsPlot.addSeries("xtt(t) Acceleration", IM_COL32(60, 180, 255, 255), 2.0f);

    // Initialize trajectory plot series (Phase space: x on horizontal, v on vertical)
    trajectoryPlot.addSeries("Phase curve (x, xt)", IM_COL32(255, 75, 75, 255), 2.0f);

    // Initialize error plot series
    errorPlot.addSeries("xs(t) Simulated", IM_COL32(0, 210, 255, 255), 2.0f);
    errorPlot.addSeries("xe(t) Exact analytical", IM_COL32(255, 220, 50, 255), 2.0f);
    errorPlot.addSeries("xs(t) - xe(t) Error", IM_COL32(255, 75, 75, 255), 2.0f);
}

void SpringSimUI::updatePlotData() {
    const auto& hist = physics.history;
    if (hist.empty()) return;

    // Build data vectors for forces
    std::vector<ImVec2> pts_f, pts_g, pts_h, pts_w;
    pts_f.reserve(hist.size());
    pts_g.reserve(hist.size());
    pts_h.reserve(hist.size());
    pts_w.reserve(hist.size());

    // Build data vectors for kinematics
    std::vector<ImVec2> pts_x, pts_v, pts_a;
    pts_x.reserve(hist.size());
    pts_v.reserve(hist.size());
    pts_a.reserve(hist.size());

    // Build data vectors for trajectory (x, v)
    std::vector<ImVec2> pts_traj;
    pts_traj.reserve(hist.size());

    // Build data vectors for error
    std::vector<ImVec2> pts_xs, pts_xe, pts_err;
    pts_xs.reserve(hist.size());
    pts_xe.reserve(hist.size());
    pts_err.reserve(hist.size());

    for (const auto& pt : hist) {
        pts_f.push_back(ImVec2(pt.t, pt.f));
        pts_g.push_back(ImVec2(pt.t, pt.g));
        pts_h.push_back(ImVec2(pt.t, pt.h));
        pts_w.push_back(ImVec2(pt.t, pt.w));

        pts_x.push_back(ImVec2(pt.t, pt.x));
        pts_v.push_back(ImVec2(pt.t, pt.v));
        pts_a.push_back(ImVec2(pt.t, pt.a));

        pts_traj.push_back(ImVec2(pt.x, pt.v));

        pts_xs.push_back(ImVec2(pt.t, pt.x));
        pts_xe.push_back(ImVec2(pt.t, pt.xe));
        pts_err.push_back(ImVec2(pt.t, pt.err));
    }

    forcesPlot.setSeriesData(0, pts_f);
    forcesPlot.setSeriesData(1, pts_g);
    forcesPlot.setSeriesData(2, pts_h);
    forcesPlot.setSeriesData(3, pts_w);

    kinematicsPlot.setSeriesData(0, pts_x);
    kinematicsPlot.setSeriesData(1, pts_v);
    kinematicsPlot.setSeriesData(2, pts_a);

    trajectoryPlot.setSeriesData(0, pts_traj);

    errorPlot.setSeriesData(0, pts_xs);
    errorPlot.setSeriesData(1, pts_xe);
    errorPlot.setSeriesData(2, pts_err);
}

void SpringSimUI::renderUI(Window& window) {
    updatePlotData();

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
            renderGraphsPanel();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

void SpringSimUI::renderControlsPanel() {
    ImGui::Spacing();

    // Playback Controls
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

    // Real-Time Telemetry
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

    // Physical Parameters
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

    // Numerical Integrator Selection
    const char* integratorNames[] = {
        "Explicit Euler",
        "Semi-Implicit Euler",
        "Central Difference (from PDF)",
        "Runge-Kutta 4 (RK4)"
    };
    int curIntegrator = static_cast<int>(physics.integrator);
    if (ImGui::Combo("Integrator", &curIntegrator, integratorNames, 4)) {
        physics.integrator = static_cast<IntegratorType>(curIntegrator);
    }

    renderSepAndSpace();

    // Free End Displacement w(t)
    ImGui::TextColored(ImVec4(0.9f, 0.4f, 0.9f, 1.0f), "Free End Function w(t)");
    const char* wFuncNames[] = {
        "Constant (f1 = A)",
        "Step (f2: 0 for t<0, A for t>=0)",
        "Square wave (f3 = sgn(A*sin(wt+phi)))",
        "Sinusoidal (f4 = A*sin(wt+phi))"
    };
    int wTypeIdx = static_cast<int>(physics.wFunc.type);
    if (ImGui::Combo("w(t) Type", &wTypeIdx, wFuncNames, 4)) {
        physics.wFunc.type = static_cast<FunctionType>(wTypeIdx);
    }
    ImGui::SliderFloat("w Amplitude A", &physics.wFunc.amplitude, -10.0f, 10.0f);
    if (physics.wFunc.type == FunctionType::Square || physics.wFunc.type == FunctionType::Sine) {
        ImGui::SliderFloat("w Frequency w", &physics.wFunc.omega, 0.1f, 15.0f);
        ImGui::SliderFloat("w Phase phi", &physics.wFunc.phi, -3.1415f, 3.1415f);
    }

    renderSepAndSpace();

    // External Force Function h(t)
    ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "Outer Field Force h(t)");
    const char* hFuncNames[] = {
        "Constant (f1 = A)",
        "Step (f2: 0 for t<0, A for t>=0)",
        "Square wave (f3 = sgn(A*sin(wt+phi)))",
        "Sinusoidal (f4 = A*sin(wt+phi))"
    };
    int hTypeIdx = static_cast<int>(physics.hFunc.type);
    if (ImGui::Combo("h(t) Type", &hTypeIdx, hFuncNames, 4)) {
        physics.hFunc.type = static_cast<FunctionType>(hTypeIdx);
    }
    ImGui::SliderFloat("h Amplitude A", &physics.hFunc.amplitude, -20.0f, 20.0f);
    if (physics.hFunc.type == FunctionType::Square || physics.hFunc.type == FunctionType::Sine) {
        ImGui::SliderFloat("h Frequency w", &physics.hFunc.omega, 0.1f, 15.0f);
        ImGui::SliderFloat("h Phase phi", &physics.hFunc.phi, -3.1415f, 3.1415f);
    }

    renderSepAndSpace();

    // 3D Visual Appearance Controls
    if (ImGui::CollapsingHeader("3D Visual & Lighting Properties")) {
        ImGui::SliderFloat("Cube Size", &cubeSize, 0.2f, 3.0f);
        ImGui::ColorEdit3("Cube Color", cubeColor);
        ImGui::ColorEdit3("Spring Color", springColor);
        ImGui::SliderFloat3("Light Position", lightPos, -20.0f, 20.0f);
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
            ImGui::Text("Position x(t), Velocity xt(t) and Acceleration xtt(t):");
            kinematicsPlot.render(ImVec2(-1.0f, 240.0f));
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

