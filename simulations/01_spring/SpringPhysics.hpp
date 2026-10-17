#pragma once
#include <vector>
#include <string>
#include <memory>
#include <cmath>

enum class IntegratorType {
    Euler = 0,
    SemiImplicitEuler = 1,
    CentralDifference = 2,
    RungeKutta4 = 3
};

enum class FunctionType {
    Constant = 0,      // f1 = A
    Step = 1,          // f2 = (t >= t_step) ? A : 0
    Square = 2,        // f3 = sgn(A * sin(omega * t + phi))
    Sine = 3,          // f4 = A * sin(omega * t + phi))
    BangBang_Ex6 = 4   // Time-optimal acceleration profile (Exercise 6)
};

struct SignalFunction {
    FunctionType type = FunctionType::Constant;
    float amplitude = 0.0f;
    float omega = 1.0f;
    float phi = 0.0f;
    float stepTime = 0.0f;

    float evaluate(float t, float m = 1.0f, float c = 1.0f, float k = 0.0f) const;
};

struct SimPoint {
    float t = 0.0f;
    float x = 0.0f;
    float v = 0.0f;
    float a = 0.0f;
    float f = 0.0f;    // spring force: c * (w - x)
    float g = 0.0f;    // damping force: -k * v
    float h = 0.0f;    // external force: h(t)
    float w = 0.0f;    // displacement of free end: w(t)
    float xe = 0.0f;   // exact analytical position
    float err = 0.0f;  // xs - xe
};

struct ConvergenceResult {
    float delta = 0.0f;
    float maxError = 0.0f;
    float rmsError = 0.0f;
};

struct ResonanceDataPoint {
    float omega = 0.0f;
    float amplitude = 0.0f;
    float phase = 0.0f;
};

struct ResonanceCurve {
    float k = 0.0f;
    std::vector<ResonanceDataPoint> points;
    float omega_r = 0.0f;
    float max_amplitude = 0.0f;
};

struct SettlingPoint {
    float k = 0.0f;
    float settlingTime = 0.0f;
};

class SpringPhysics {
public:
    // Physical Parameters
    float m = 1.0f;       // mass
    float c = 1.0f;       // spring stiffness (c > 0)
    float k = 0.1f;       // damping coefficient (k >= 0)
    float x0 = 1.0f;      // initial position
    float v0 = 0.0f;      // initial velocity
    float delta = 0.01f;  // integration step

    // Integrator
    IntegratorType integrator = IntegratorType::SemiImplicitEuler;

    // Excitation functions
    SignalFunction wFunc; // w(t) - displacement of free end
    SignalFunction hFunc; // h(t) - external direct force

    // Simulation Execution State
    bool isRunning = true;
    float simTimeSpeed = 1.0f;
    float maxHistoryTime = 60.0f; // maximum duration of history to keep in seconds
    size_t maxHistoryPoints = 12000;

    // Current State
    float t = 0.0f;
    float x = 0.0f;
    float v = 0.0f;
    float a = 0.0f;
    float f = 0.0f;
    float g = 0.0f;
    float h = 0.0f;
    float w = 0.0f;
    float xe = 0.0f;
    float err = 0.0f;

    // Internal state for central difference
    float x_prev = 0.0f;
    bool hasPrev = false;

    // Recorded history
    std::vector<SimPoint> history;

    // Analytical exact comparison
    float maxAbsoluteError = 0.0f;
    float sumSquaredError = 0.0f;
    size_t errorCount = 0;

public:
    SpringPhysics();

    void reset();
    void step();
    void update(float realDt);

    // Exact analytical solution evaluator
    float computeExactSolution(float tQuery) const;
    float computeExactVelocity(float tQuery) const;

    // Helpers for exercises
    void setupExercise1();
    std::vector<ConvergenceResult> runExercise1ConvergenceTest(IntegratorType method, const std::vector<float>& deltas, float testDuration = 10.0f);

    void setupExercise2();
    float runExercise2CriticalDampingSearch(float& theoreticalK, float maxT = 15.0f);

    void setupExercise3();
    std::vector<SettlingPoint> runExercise3SettlingSweep(float tolerance = 0.02f, float maxT = 20.0f, float kMin = 0.2f, float kMax = 3.0f, int steps = 40);

    void setupExercise4();
    float computeExercise4SettlingTime(float threshold = 0.1f) const;

    void setupExercise5();
    std::vector<ResonanceCurve> computeExercise5ResonanceCurves(const std::vector<float>& kValues, float omegaMin = 0.1f, float omegaMax = 2.5f, int steps = 150) const;

    void setupExercise6();
    static void getExercise6OptimalTrajectory(float t, float& x_out, float& v_out, float& a_out, float& w_out, float m = 1.0f, float c = 1.0f, float k = 0.1f);

private:
    float computeAcceleration(float curX, float curV, float curT) const;
    void recordPoint();
};
