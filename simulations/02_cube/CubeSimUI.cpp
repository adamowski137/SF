#include "CubeSimUI.hpp"
#include "core/Window.hpp"
#include "core/Camera.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <cstdio>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

CubeSimUI::CubeSimUI(int width, CubePhysics& phys, Camera& cam)
    : UI(width), physics(phys), camera(cam),
      trajectoryPlot("Rzut trajektorii w plaszczyznie poziomej (X-Z)", true),
      energyPlot("Energia ukladu w czasie"),
      angularVelPlot("Predkosc katowa W w ukladzie bryly"),
      inclinationPlot("Wychylenie przekatnej theta(t)")
{
    // Configure series
    trajectoryPlot.addSeries("Koniec przekatnej (X, Z)", IM_COL32(0, 220, 255, 255), 2.0f);

    energyPlot.addSeries("E_rot Kinetyczna", IM_COL32(255, 170, 40, 255), 2.0f);
    energyPlot.addSeries("E_pot Potencjalna", IM_COL32(60, 220, 100, 255), 2.0f);
    energyPlot.addSeries("E_tot Calkowita", IM_COL32(255, 60, 60, 255), 2.5f);

    angularVelPlot.addSeries("Wx", IM_COL32(255, 80, 80, 255), 2.0f);
    angularVelPlot.addSeries("Wy", IM_COL32(80, 230, 100, 255), 2.0f);
    angularVelPlot.addSeries("Wz", IM_COL32(80, 170, 255, 255), 2.0f);

    inclinationPlot.addSeries("theta [deg]", IM_COL32(255, 215, 0, 255), 2.0f);
}

void CubeSimUI::updatePlotData() {
    const auto& hist = physics.history;
    if (hist.empty()) return;

    // Trajectory plot: (X, Z)
    auto& trajPts = trajectoryPlot.series[0].points;
    trajPts.clear();
    trajPts.reserve(physics.trajectory.size());
    for (const auto& p : physics.trajectory) {
        trajPts.push_back(ImVec2(p.x, p.z));
    }

    // Energy plot
    auto& eKinPts = energyPlot.series[0].points;
    auto& ePotPts = energyPlot.series[1].points;
    auto& eTotPts = energyPlot.series[2].points;
    eKinPts.clear(); ePotPts.clear(); eTotPts.clear();
    eKinPts.reserve(hist.size());
    ePotPts.reserve(hist.size());
    eTotPts.reserve(hist.size());

    // Angular velocity plot
    auto& wxPts = angularVelPlot.series[0].points;
    auto& wyPts = angularVelPlot.series[1].points;
    auto& wzPts = angularVelPlot.series[2].points;
    wxPts.clear(); wyPts.clear(); wzPts.clear();
    wxPts.reserve(hist.size());
    wyPts.reserve(hist.size());
    wzPts.reserve(hist.size());

    // Inclination plot
    auto& thetaPts = inclinationPlot.series[0].points;
    thetaPts.clear();
    thetaPts.reserve(hist.size());

    for (const auto& pt : hist) {
        eKinPts.push_back(ImVec2(pt.t, pt.eKin));
        ePotPts.push_back(ImVec2(pt.t, pt.ePot));
        eTotPts.push_back(ImVec2(pt.t, pt.eTot));

        wxPts.push_back(ImVec2(pt.t, pt.W.x));
        wyPts.push_back(ImVec2(pt.t, pt.W.y));
        wzPts.push_back(ImVec2(pt.t, pt.W.z));

        thetaPts.push_back(ImVec2(pt.t, pt.thetaDeg));
    }
}

void CubeSimUI::renderUI(Window& window) {
    const ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse;

    ImGui::SetNextWindowPos(ImVec2(static_cast<float>(window.getViewportWidth()), 0.0f));
    ImGui::SetNextWindowSize(ImVec2(static_cast<float>(window.getWidth() - window.getViewportWidth()),
                                   static_cast<float>(window.getHeight())));

    ImGui::Begin("CubeSimControlPanel", nullptr, windowFlags);

    ImGui::TextColored(ImVec4(1.0f, 0.82f, 0.2f, 1.0f), "LAB 2: BACZEK (SZESCIAN W NARROZU)");
    ImGui::TextDisabled("Symulacja ruchu obrotowego szescianu (Euler + RK4)");
    ImGui::Separator();

    renderSimulationControls();

    if (ImGui::BeginTabBar("CubeSimTabs")) {
        if (ImGui::BeginTabItem("Wizualizacja (Krok 1)")) {
            renderVisualizationTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Parametry i Fizyka (Krok 2)")) {
            renderPhysicsParametersTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Wykresy i Telemetria")) {
            renderTelemetryAndPlotsTab();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}

void CubeSimUI::renderSimulationControls() {
    ImGui::Spacing();

    // Play / Pause / Step / Reset buttons
    if (physics.isRunning) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.2f, 0.2f, 1.0f));
        if (ImGui::Button("Pauza [Space]", ImVec2(110, 28))) {
            physics.isRunning = false;
        }
        ImGui::PopStyleColor();
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.3f, 1.0f));
        if (ImGui::Button("Start [Space]", ImVec2(110, 28))) {
            physics.isRunning = true;
        }
        ImGui::PopStyleColor();
    }

    ImGui::SameLine();
    if (ImGui::Button("Krok", ImVec2(70, 28))) {
        physics.isRunning = false;
        physics.step();
        updatePlotData();
    }

    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.4f, 0.6f, 1.0f));
    if (ImGui::Button("Reset", ImVec2(75, 28))) {
        physics.reset();
        updatePlotData();
    }
    ImGui::PopStyleColor();

    ImGui::SameLine();
    ImGui::Text("t: %.2f s | FPS: %.0f", physics.t, ImGui::GetIO().Framerate);

    // Speed multiplier and physics dt
    ImGui::SliderFloat("Predkosc symulacji", &physics.simTimeSpeed, 0.1f, 4.0f, "%.1fx");
    ImGui::SliderFloat("Krok calkowania dt", &physics.delta, 0.0002f, 0.005f, "%.4f s");

    // Presets
    const char* presets[] = {
        "1. Precesja regularna (bączek symetryczny)",
        "2. Pętle nutacji (z zaburzeniem poprzecznym)",
        "3. Swobodny obrót bez grawitacji (Poinsot)",
        "4. Wahadło fizyczne (brak wirowania)",
        "5. Bączek śpiący (duża prędkość, pionowo)"
    };
    if (ImGui::Combo("Scenariusz (Preset)", &activePresetIdx, presets, IM_ARRAYSIZE(presets))) {
        physics.loadPreset(static_cast<CubeSimPreset>(activePresetIdx));
        updatePlotData();
    }

    ImGui::Separator();
}

void CubeSimUI::renderVisualizationTab() {
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.4f, 0.85f, 1.0f, 1.0f), "1. WYSWIETLANIE NA ZYCZENIE (Krok 1.1)");

    ImGui::Checkbox("Szescian (bryla)", &visual.showCube);
    ImGui::SameLine(180);
    ImGui::Checkbox("Krawedzie (wireframe)", &visual.showWireframe);

    ImGui::Checkbox("Przekatna szescianu (z narroza)", &visual.showDiagonal);
    ImGui::Checkbox("Trajektoria swobodnego konca", &visual.showTrajectory);

    ImGui::Checkbox("Kierunek grawitacji (wektor)", &visual.showGravityVector);
    ImGui::SameLine(230);
    ImGui::Checkbox("Polprzezroczysta plaszczyzna", &visual.showPerpendicularPlane);

    ImGui::Checkbox("Punkty narozne (narroze i koniec)", &visual.showMarkers);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.4f, 0.85f, 1.0f, 1.0f), "PARAMETRY TRAJEKTORII (Krok 1.3)");

    if (ImGui::SliderInt("Dlugosc trajektorii n", &physics.trajectoryLengthN, 20, 5000)) {
        if (static_cast<int>(physics.trajectory.size()) > physics.trajectoryLengthN) {
            size_t excess = physics.trajectory.size() - physics.trajectoryLengthN;
            physics.trajectory.erase(physics.trajectory.begin(), physics.trajectory.begin() + excess);
        }
    }
    ImGui::Text("Aktualnie zgromadzonych punktow: %zu", physics.trajectory.size());

    if (ImGui::Button("Wyczysc trajektorie")) {
        physics.clearTrajectory();
        updatePlotData();
    }
}

void CubeSimUI::renderPhysicsParametersTab() {
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.4f, 0.85f, 1.0f, 1.0f), "WYMIARY I GESTOSC SZESCIANU (Krok 1.3)");

    ImGui::Checkbox("Jednakowe wymiary (Szescian foremny)", &physics.uniformCube);

    bool dimChanged = false;
    if (physics.uniformCube) {
        float side = physics.a;
        if (ImGui::SliderFloat("Dlugosc boku a [m]", &side, 0.2f, 3.0f, "%.2f m")) {
            physics.setDimensions(side, side, side);
            dimChanged = true;
        }
    } else {
        float dim[3] = { physics.a, physics.b, physics.c };
        if (ImGui::SliderFloat3("Wymiary a, b, c [m]", dim, 0.2f, 3.0f, "%.2f m")) {
            physics.setDimensions(dim[0], dim[1], dim[2]);
            dimChanged = true;
        }
    }

    float rho = physics.rho;
    if (ImGui::SliderFloat("Gestosc rho [kg/m^3]", &rho, 10.0f, 5000.0f, "%.0f kg/m^3")) {
        physics.setDensity(rho);
        dimChanged = true;
    }

    if (dimChanged) {
        updatePlotData();
    }

    ImGui::Text("Masa calkowita M: %.2f kg", physics.mass);
    ImGui::Text("Srodek masy (w bryle): (%.2f, %.2f, %.2f) m", physics.r_cm.x, physics.r_cm.y, physics.r_cm.z);
    ImGui::Text("Dlugosc przekatnej: %.3f m", glm::length(physics.getDiagonalBody()));

    ImGui::Spacing();
    ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.3f, 1.0f), "TENSOR BEZWŁADNOSCI I WZGLĘDEM NAROŻA:");
    ImGui::TextDisabled("Automatycznie przeliczany z wymiarów i gęstości:");

    // Display 3x3 inertia tensor matrix
    const glm::mat3& matI = physics.I;
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.12f, 0.14f, 0.18f, 1.0f));
    ImGui::BeginChild("InertiaMatrix", ImVec2(0, 95), true);
    ImGui::Columns(3, "I_cols", false);
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            ImGui::Text("%9.4f", matI[col][row]);
            ImGui::NextColumn();
        }
    }
    ImGui::Columns(1);
    ImGui::EndChild();
    ImGui::PopStyleColor();

    ImGui::Text("Moment wzdluz przekatnej (I_diag): %.4f kg*m^2", physics.I_diag);
    ImGui::Text("Moment poprzeczny (I_perp):       %.4f kg*m^2", physics.I_perp);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.4f, 0.85f, 1.0f, 1.0f), "WYCHYLENIE SZESCIANU I PRĘDKOŚĆ (Krok 1.3)");

    ImGui::SliderFloat("Wychylenie przekatnej theta [deg]", &physics.initThetaDeg, 0.0f, 179.0f, "%.1f deg");
    ImGui::SliderFloat("Azymut przekatnej phi [deg]", &physics.initPhiDeg, 0.0f, 360.0f, "%.1f deg");
    ImGui::SliderFloat("Obrot wlasny psi [deg]", &physics.initPsiDeg, 0.0f, 360.0f, "%.1f deg");

    ImGui::Spacing();
    ImGui::SliderFloat("Predkosc katowa wokol przekatnej [rad/s]", &physics.initOmegaDiag, -150.0f, 150.0f, "%.1f rad/s");
    ImGui::SliderFloat("Zaburzenie poprzeczne W_perp [rad/s]", &physics.initOmegaPerp, -30.0f, 30.0f, "%.1f rad/s");

    if (ImGui::Button("Zastosuj do stanu poczatkowego i Reset")) {
        physics.reset();
        updatePlotData();
    }
    ImGui::SameLine();
    if (ImGui::Button("Ustaw orientacje teraz")) {
        physics.applyOrientationFromDiagonal(physics.initThetaDeg, physics.initPhiDeg, physics.initPsiDeg);
        physics.clearTrajectory();
    }
    ImGui::SameLine();
    if (ImGui::Button("Ustaw predkosc teraz")) {
        physics.applyAngularVelocityAroundDiagonal(physics.initOmegaDiag, physics.initOmegaPerp);
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.4f, 0.85f, 1.0f, 1.0f), "GRAWITACJA I OPORY (Krok 1.3)");

    ImGui::Checkbox("Grawitacja wlaczona", &physics.gravityEnabled);
    if (physics.gravityEnabled) {
        ImGui::SliderFloat("Przyspieszenie g [m/s^2]", &physics.gValue, 0.0f, 30.0f, "%.2f m/s^2");
    }
    ImGui::SliderFloat("Tlumienie obrotu (drag)", &physics.dampingCoeff, 0.0f, 0.2f, "%.4f");
}

void CubeSimUI::renderTelemetryAndPlotsTab() {
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.4f, 0.85f, 1.0f, 1.0f), "TELEMETRIA FIZYCZNA NA ZYWO");

    glm::vec3 tip = physics.getTipWorldPosition();
    float curTheta = physics.getCurrentInclinationDeg();
    ImGui::Text("Koniec przekatnej: (%.3f, %.3f, %.3f) m", tip.x, tip.y, tip.z);
    ImGui::Text("Biezace wychylenie theta: %.2f deg", curTheta);

    glm::vec3 wBody = physics.W;
    glm::vec3 wWorld = physics.getAngularVelocityWorld();
    ImGui::Text("Predkosc katowa W (bryla):  (%.2f, %.2f, %.2f) rad/s  |W| = %.2f", wBody.x, wBody.y, wBody.z, glm::length(wBody));
    ImGui::Text("Predkosc katowa W (scena):  (%.2f, %.2f, %.2f) rad/s", wWorld.x, wWorld.y, wWorld.z);

    glm::vec3 lWorld = physics.getAngularMomentumWorld();
    ImGui::Text("Moment pedu L (scena):      (%.2f, %.2f, %.2f) kg*m^2/s  Ly = %.3f", lWorld.x, lWorld.y, lWorld.z, lWorld.y);

    float eKin = physics.getKineticEnergy();
    float ePot = physics.getPotentialEnergy();
    float eTot = physics.getTotalEnergy();
    ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.2f, 1.0f), "Energia: E_kin = %.2f J | E_pot = %.2f J | E_tot = %.2f J", eKin, ePot, eTot);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.4f, 0.85f, 1.0f, 1.0f), "WYKRESY");

    if (ImGui::CollapsingHeader("1. Rzut trajektorii konca przekatnej (X-Z)", ImGuiTreeNodeFlags_DefaultOpen)) {
        trajectoryPlot.render(ImVec2(-1.0f, 210.0f));
    }

    if (ImGui::CollapsingHeader("2. Zasada zachowania energii w czasie (E_kin, E_pot, E_tot)", ImGuiTreeNodeFlags_DefaultOpen)) {
        energyPlot.render(ImVec2(-1.0f, 180.0f));
    }

    if (ImGui::CollapsingHeader("3. Predkosc katowa W(t)")) {
        angularVelPlot.render(ImVec2(-1.0f, 180.0f));
    }

    if (ImGui::CollapsingHeader("4. Wychylenie przekatnej theta(t)")) {
        inclinationPlot.render(ImVec2(-1.0f, 180.0f));
    }
}

