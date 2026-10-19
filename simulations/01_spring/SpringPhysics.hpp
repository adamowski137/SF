#pragma once
#include <vector>
#include <string>
#include <cmath>

enum class FunctionType {
    Constant = 0,
    Step = 1,
    Square = 2,
    Sine = 3,
    Exercise6 = 4
};

struct SignalFunction {
    FunctionType type = FunctionType::Constant;
    float amplitude = 0.0f;
    float omega = 1.0f;
    float phi = 0.0f;
    float stepTime = 0.0f;

    float evaluate(float t, float m = 1.0f, float c = 1.0f, float k = 0.1f) const;
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
    float m = 1.0f;
    float c = 1.0f;
    float k = 0.1f;
    float x0 = 1.0f;
    float v0 = 0.0f;
    float delta = 0.01f;

    SignalFunction wFunc; // w(t) - displacement of free end
    SignalFunction hFunc; // h(t) - external direct force

    bool isRunning = true;
    float simTimeSpeed = 1.0f;
    size_t maxHistoryPoints = 12000;

    float t = 0.0f;
    float x = 0.0f;
    float v = 0.0f;
    float a = 0.0f;
    float f = 0.0f;    // spring force: c * (w - x)
    float g = 0.0f;    // damping force: -k * v
    float h = 0.0f;    // external force: h(t)
    float w = 0.0f;    // displacement of free end: w(t)
    float xe = 0.0f;
    float err = 0.0f;

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
