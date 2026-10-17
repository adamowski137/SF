#include "SpringSimUI.hpp"
#include "core/Window.hpp"
#include <cstring>
#include <cmath>

SpringSimUI::SpringSimUI(int width, SpringPhysics& phys, float& inCubeSize, float* inCubeColor, float* inLightPos)
    : UI(width), physics(phys), cubeSize(inCubeSize), cubeColor(inCubeColor), lightPos(inLightPos),
      forcesPlot("Forces Plot"),
      kinematicsPlot("Kinematics Plot"),
      trajectoryPlot("Phase Space Trajectory", true),
      errorPlot("Simulation vs Exact Solution"),
      ex4Plot("Exercise 4: Tracking Error"),
      resonanceAmpPlot("Resonance: Amplitude A(w)"),
      resonancePhasePlot("Resonance: Phase phi(w)"),
      settlingPlot("Exercise 3: Settling Time T(k)")
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

    // Initialize Exercise 4 plot series
    ex4Plot.addSeries("x(t)", IM_COL32(0, 210, 255, 255), 2.0f);
    ex4Plot.addSeries("w(t)", IM_COL32(255, 170, 40, 255), 2.0f);
    ex4Plot.addSeries("w(t) - x(t)", IM_COL32(255, 80, 220, 255), 2.0f);

    // Initialize Resonance amplitude plot series
    resonanceAmpPlot.addSeries("k = 1.0", IM_COL32(80, 160, 255, 255), 2.0f);
    resonanceAmpPlot.addSeries("k = 0.5", IM_COL32(60, 220, 100, 255), 2.0f);
    resonanceAmpPlot.addSeries("k = 0.1", IM_COL32(255, 220, 50, 255), 2.0f);
    resonanceAmpPlot.addSeries("k = 0.01", IM_COL32(255, 70, 70, 255), 2.0f);

    // Initialize Resonance phase plot series
    resonancePhasePlot.addSeries("k = 1.0", IM_COL32(80, 160, 255, 255), 2.0f);
    resonancePhasePlot.addSeries("k = 0.5", IM_COL32(60, 220, 100, 255), 2.0f);
    resonancePhasePlot.addSeries("k = 0.1", IM_COL32(255, 220, 50, 255), 2.0f);
    resonancePhasePlot.addSeries("k = 0.01", IM_COL32(255, 70, 70, 255), 2.0f);

    // Initialize Settling time plot series
    settlingPlot.addSeries("T_settle(k)", IM_COL32(255, 200, 50, 255), 2.0f);
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

    // Build data vectors for Ex4
    std::vector<ImVec2> pts_ex4_x, pts_ex4_w, pts_ex4_diff;
    pts_ex4_x.reserve(hist.size());
    pts_ex4_w.reserve(hist.size());
    pts_ex4_diff.reserve(hist.size());

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

        pts_ex4_x.push_back(ImVec2(pt.t, pt.x));
        pts_ex4_w.push_back(ImVec2(pt.t, pt.w));
        pts_ex4_diff.push_back(ImVec2(pt.t, pt.w - pt.x));
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

    ex4Plot.setSeriesData(0, pts_ex4_x);
    ex4Plot.setSeriesData(1, pts_ex4_w);
    ex4Plot.setSeriesData(2, pts_ex4_diff);
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

        if (ImGui::BeginTabItem("Lab Exercises (1-6)")) {
            renderExercisesPanel();
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
        "Sinusoidal (f4 = A*sin(wt+phi))",
        "Optimal Bang-Bang (Ex 6)"
    };
    int wTypeIdx = static_cast<int>(physics.wFunc.type);
    if (ImGui::Combo("w(t) Type", &wTypeIdx, wFuncNames, 5)) {
        physics.wFunc.type = static_cast<FunctionType>(wTypeIdx);
    }
    if (physics.wFunc.type != FunctionType::BangBang_Ex6) {
        ImGui::SliderFloat("w Amplitude A", &physics.wFunc.amplitude, -10.0f, 10.0f);
        if (physics.wFunc.type == FunctionType::Square || physics.wFunc.type == FunctionType::Sine) {
            ImGui::SliderFloat("w Frequency w", &physics.wFunc.omega, 0.1f, 15.0f);
            ImGui::SliderFloat("w Phase phi", &physics.wFunc.phi, -3.1415f, 3.1415f);
        }
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

void SpringSimUI::renderExercisesPanel() {
    ImGui::Spacing();

    if (ImGui::BeginTabBar("ExercisesTabs")) {
        // Exercise 1: Role of parameters & numerical delta accuracy
        if (ImGui::BeginTabItem("Ex 1: Parameters & Delta")) {
            ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "Zadanie 1: Rola parametrow ukladu");
            ImGui::TextWrapped("Niech w(t) = 0, x(0) = 1, v(0) = 0. Obserwuj ruch ukladu az do osiagniecia rownowagi.\nSprawdz wplyw kroku calkowania Delta na dokladnosc obliczen: xs(t) - xe(t).");
            ImGui::Spacing();

            if (ImGui::Button("Load Exercise 1 Preset")) {
                physics.setupExercise1();
            }

            ImGui::Separator();
            errorPlot.render(ImVec2(-1.0f, 200.0f));
            ImGui::Text("Max Absolute Error: %.6f", physics.maxAbsoluteError);

            ImGui::Spacing();
            if (ImGui::Button("Run Delta Convergence Analysis (Euler vs Central vs RK4)")) {
                std::vector<float> testDeltas = { 0.1f, 0.05f, 0.02f, 0.01f, 0.005f, 0.001f };
                ex1Results = physics.runExercise1ConvergenceTest(physics.integrator, testDeltas, 10.0f);
                hasEx1Results = true;
            }

            if (hasEx1Results) {
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), "Convergence Test Results (over T = 10s):");
                ImGui::Columns(3, "conv_table", true);
                ImGui::Text("Step Delta"); ImGui::NextColumn();
                ImGui::Text("Max |xs - xe|"); ImGui::NextColumn();
                ImGui::Text("RMS Error"); ImGui::NextColumn();
                ImGui::Separator();
                for (const auto& res : ex1Results) {
                    ImGui::Text("%.4f", res.delta); ImGui::NextColumn();
                    ImGui::Text("%.6f", res.maxError); ImGui::NextColumn();
                    ImGui::Text("%.6f", res.rmsError); ImGui::NextColumn();
                }
                ImGui::Columns(1);
            }
            ImGui::EndTabItem();
        }

        // Exercise 2: Critical Damping
        if (ImGui::BeginTabItem("Ex 2: Critical Damping")) {
            ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "Zadanie 2: Tlumienie krytyczne");
            ImGui::TextWrapped("Dla x(0) = 1, v(0) = 0, m = c = 1 znajdz doswiadczalnie wartosc krytyczna k_kr:\ndla wszystkich k < k_kr uklad oscyluje wokol rownowagi (przechodzi przez 0).");
            ImGui::Spacing();

            if (ImGui::Button("Load Exercise 2 Preset")) {
                physics.setupExercise2();
            }

            ImGui::Spacing();
            float thK = 2.0f * std::sqrt(physics.m * physics.c);
            ImGui::Text("Theoretical Critical Damping: k_kr = 2*sqrt(m*c) = %.4f", thK);

            if (physics.k < thK) {
                ImGui::TextColored(ImVec4(0.2f, 0.9f, 0.3f, 1.0f), "Status: UNDERDAMPED (Oscillates around equilibrium, k < k_kr)");
            } else if (std::abs(physics.k - thK) < 0.01f) {
                ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.2f, 1.0f), "Status: CRITICALLY DAMPED (Fastest monotonic return)");
            } else {
                ImGui::TextColored(ImVec4(0.9f, 0.4f, 0.2f, 1.0f), "Status: OVERDAMPED (Slow exponential return, no oscillations)");
            }

            ImGui::Spacing();
            if (ImGui::Button("Find Experimental k_kr via Bisection Search")) {
                ex2ExpK = physics.runExercise2CriticalDampingSearch(ex2TheoryK, 15.0f);
                hasEx2Results = true;
            }

            if (hasEx2Results) {
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), "Experimental Finding:");
                ImGui::BulletText("Experimental k_kr: %.4f", ex2ExpK);
                ImGui::BulletText("Theoretical  k_kr: %.4f", ex2TheoryK);
                ImGui::BulletText("Difference:        %.6f", std::abs(ex2ExpK - ex2TheoryK));
            }
            ImGui::EndTabItem();
        }

        // Exercise 3: Displacement Step Response & Minimum Settling Time
        if (ImGui::BeginTabItem("Ex 3: Settling Time")) {
            ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "Zadanie 3: Funkcja przemieszczenia (Skok w(t) = f2)");
            ImGui::TextWrapped("Dla x(0)=0, v(0)=0, m=c=1 oraz skoku w(t)=f2 znajdz tlumienie k minimalizujace czas osiagniecia stanu rownowagi.");
            ImGui::Spacing();

            if (ImGui::Button("Load Exercise 3 Preset")) {
                physics.setupExercise3();
            }

            ImGui::Spacing();
            static float tolPercent = 2.0f; // 2% band
            ImGui::SliderFloat("Settling Band Tolerance (%)", &tolPercent, 0.5f, 5.0f);

            if (ImGui::Button("Run Settling Time Sweep T_settle(k)")) {
                ex3Results = physics.runExercise3SettlingSweep(tolPercent * 0.01f, 20.0f, 0.2f, 3.0f, 40);
                hasEx3Results = true;

                // Find minimum settling time point
                float minT = 1e9f;
                float bestK = 0.0f;
                std::vector<ImVec2> pts;
                for (const auto& sp : ex3Results) {
                    pts.push_back(ImVec2(sp.k, sp.settlingTime));
                    if (sp.settlingTime < minT) {
                        minT = sp.settlingTime;
                        bestK = sp.k;
                    }
                }
                ex3OptimalK = bestK;
                ex3MinSettlingT = minT;
                settlingPlot.setSeriesData(0, pts);
            }

            if (hasEx3Results) {
                ImGui::Spacing();
                settlingPlot.render(ImVec2(-1.0f, 200.0f));
                ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f),
                    "Optimal Damping: k* = %.3f with minimal T_settle = %.3f s",
                    ex3OptimalK, ex3MinSettlingT);
                ImGui::Text("Comparison with critical damping k_kr = 2.000:");
                ImGui::BulletText("For 2%% criterion, optimal k* ~ 1.4-1.6 is slightly underdamped.");
                ImGui::BulletText("For strict monotonicity (no overshoot), k = k_kr = 2.0.");
            }
            ImGui::EndTabItem();
        }

        // Exercise 4: Repeated displacement (Square wave f3)
        if (ImGui::BeginTabItem("Ex 4: Square Wave")) {
            ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "Zadanie 4: Przemieszczenie wielokrotne (w(t) = f3)");
            ImGui::TextWrapped("Dla x(0)=1, v(0)=0, m=c=1, w(t)=f3 (A=1, phi=0), znajdz czas T spelniajacy |w(t) - x(t)| < 0.1 dla t > T.\nFunkcje x(t), w(t) i w(t)-x(t) na jednym wspolnym wykresie:");
            ImGui::Spacing();

            if (ImGui::Button("Load Exercise 4 Preset")) {
                physics.setupExercise4();
            }

            ImGui::Spacing();
            ex4Plot.render(ImVec2(-1.0f, 220.0f));

            float T_ex4 = physics.computeExercise4SettlingTime(0.1f);
            if (T_ex4 > 0.0f) {
                ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f),
                    "Settling Time T (where |w(t) - x(t)| < 0.1): %.3f s", T_ex4);
            } else {
                ImGui::Text("Settling condition |w - x| < 0.1 not yet reached or in transition.");
            }
            ImGui::EndTabItem();
        }

        // Exercise 5: Resonance
        if (ImGui::BeginTabItem("Ex 5: Resonance")) {
            ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "Zadanie 5: Rezonans");
            ImGui::TextWrapped("Wymuszenie harmoniczne w(t) = A0*sin(wt+phi0), A0=1, phi0=0. Po zaniku stanu przejsciowego x(t) = A*sin(wt+phi).\nDla k = 1, 0.5, 0.1, 0.01 narysuj A(w) oraz phi(w), znajdz czestotliwosc rezonansowa wr.");
            ImGui::Spacing();

            if (ImGui::Button("Load Exercise 5 Simulation")) {
                physics.setupExercise5();
            }

            ImGui::Spacing();
            if (ImGui::Button("Compute Resonance Curves A(w) and phi(w)")) {
                std::vector<float> kList = { 1.0f, 0.5f, 0.1f, 0.01f };
                ex5Curves = physics.computeExercise5ResonanceCurves(kList, 0.1f, 2.5f, 200);
                hasEx5Results = true;

                for (size_t i = 0; i < ex5Curves.size(); i++) {
                    std::vector<ImVec2> ptsA, ptsPhi;
                    for (const auto& pt : ex5Curves[i].points) {
                        ptsA.push_back(ImVec2(pt.omega, pt.amplitude));
                        ptsPhi.push_back(ImVec2(pt.omega, pt.phase));
                    }
                    resonanceAmpPlot.setSeriesData(static_cast<int>(i), ptsA);
                    resonancePhasePlot.setSeriesData(static_cast<int>(i), ptsPhi);
                }
            }

            if (hasEx5Results) {
                ImGui::Spacing();
                ImGui::Text("Amplitude Response A(w):");
                resonanceAmpPlot.render(ImVec2(-1.0f, 190.0f));

                ImGui::Spacing();
                ImGui::Text("Phase Response phi(w):");
                resonancePhasePlot.render(ImVec2(-1.0f, 170.0f));

                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), "Resonance Peaks wr = sqrt(c/m - k^2/(2m^2)):");
                for (const auto& crv : ex5Curves) {
                    ImGui::BulletText("k = %.2f -> wr = %.4f rad/s, Peak A(wr) = %.2f",
                        crv.k, crv.omega_r, crv.max_amplitude);
                }
            }
            ImGui::EndTabItem();
        }

        // Exercise 6: Time-Optimal Control with Acceleration Constraint
        if (ImGui::BeginTabItem("Ex 6: Time-Optimal Control")) {
            ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "Zadanie 6: Optymalizacja czasu z |xtt| <= 1");
            ImGui::TextWrapped("Znajdz w(t) przenoszace mase z x(0)=0, v(0)=0 do x(T)=1, v(T)=0 w najkrotszym czasie przy |xtt(t)| <= 1.");
            ImGui::Spacing();

            if (ImGui::Button("Load Exercise 6 Optimal Bang-Bang Trajectory")) {
                physics.setupExercise6();
            }

            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), "Theoretical Derivation (Pontryagin Bang-Bang):");
            ImGui::BulletText("Acceleration: a(t) = +1 for t in [0, 1), and -1 for t in [1, 2]");
            ImGui::BulletText("Velocity:     v(t) = t for [0, 1), and 2 - t for [1, 2]");
            ImGui::BulletText("Position:     x(t) = 0.5*t^2 for [0, 1), and -0.5*t^2 + 2t - 1 for [1, 2]");
            ImGui::BulletText("Optimal Time: T = 2.0 seconds (fastest possible!)");
            ImGui::BulletText("Control w(t): w(t) = x(t) + (m/c)*a(t) + (k/c)*v(t)");

            ImGui::Spacing();
            kinematicsPlot.render(ImVec2(-1.0f, 220.0f));

            ImGui::Text("Current State at t = %.2f s: x = %.3f, v = %.3f, a = %.3f",
                physics.t, physics.x, physics.v, physics.a);
            if (std::abs(physics.a) <= 1.005f) {
                ImGui::TextColored(ImVec4(0.2f, 0.9f, 0.2f, 1.0f), "Constraint |xtt(t)| <= 1.0 is SATISFIED!");
            } else {
                ImGui::TextColored(ImVec4(0.9f, 0.2f, 0.2f, 1.0f), "Constraint violated!");
            }
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
}
