#pragma once
#include <vector>
#include <string>
#include <cmath>

enum class FunctionType {
    Constant = 0,      // f1 = A
    Step = 1,          // f2 = (t >= t_step) ? A : 0
    Square = 2,        // f3 = sgn(A * sin(omega * t + phi))
    Sine = 3           // f4 = A * sin(omega * t + phi))
};

struct SignalFunction {
    FunctionType type = FunctionType::Constant;
    float amplitude = 0.0f;
    float omega = 1.0f;
    float phi = 0.0f;
    float stepTime = 0.0f;

    float evaluate(float t) const;
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

class SpringPhysics {
public:
    // Physical Parameters
    float m = 1.0f;       // mass
    float c = 1.0f;       // spring stiffness (c > 0)
    float k = 0.1f;       // damping coefficient (k >= 0)
    float x0 = 1.0f;      // initial position
    float v0 = 0.0f;      // initial velocity
    float delta = 0.01f;  // integration step

    // Excitation functions
    SignalFunction wFunc; // w(t) - displacement of free end
    SignalFunction hFunc; // h(t) - external direct force

    // Simulation Execution State
    bool isRunning = true;
    float simTimeSpeed = 1.0f;
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

    // Recorded history
    std::vector<SimPoint> history;

    // Analytical exact comparison
    float maxAbsoluteError = 0.0f;

public:
    SpringPhysics();

    void reset();
    void step();
    void update(float realDt);

    // Exact analytical solution evaluator
    float computeExactSolution(float tQuery) const;

private:
    float timeAccumulator = 0.0f;

    void updateDerivedState();
    void recordPoint();
};
