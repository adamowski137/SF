#include "SpringPhysics.hpp"
#include <algorithm>
#include <cmath>

float SignalFunction::evaluate(float t) const {
    switch (type) {
        case FunctionType::Constant:
            return amplitude;

        case FunctionType::Step:
            return (t >= stepTime) ? amplitude : 0.0f;

        case FunctionType::Square: {
            float val = amplitude * std::sin(omega * t + phi);
            return (val > 1e-6f) ? 1.0f : ((val < -1e-6f) ? -1.0f : 0.0f);
        }

        case FunctionType::Sine:
            return amplitude * std::sin(omega * t + phi);

        default:
            return 0.0f;
    }
}

SpringPhysics::SpringPhysics() {
    reset();
}

void SpringPhysics::updateDerivedState() {
    w = wFunc.evaluate(t);
    h = hFunc.evaluate(t);
    f = c * (w - x);
    g = -k * v;
    a = (m > 1e-6f) ? ((f + g + h) / m) : 0.0f;

    xe = computeExactSolution(t);
    err = x - xe;

    maxAbsoluteError = std::max(maxAbsoluteError, std::abs(err));
}

void SpringPhysics::reset() {
    t = 0.0f;
    x = x0;
    v = v0;
    maxAbsoluteError = 0.0f;
    timeAccumulator = 0.0f;

    updateDerivedState();

    history.clear();
    history.reserve(2000);
    recordPoint();
}

void SpringPhysics::step() {
    if (delta <= 1e-6f) return;

    // Explicit Euler integration
    x += delta * v;
    v += delta * a;
    t += delta;

    updateDerivedState();
    recordPoint();
}

void SpringPhysics::update(float realDt) {
    if (!isRunning) return;

    float effectiveDt = realDt * simTimeSpeed;
    if (effectiveDt <= 0.0f) return;

    timeAccumulator += effectiveDt;

    // Fixed timestep accumulation
    int stepsTaken = 0;
    while (timeAccumulator >= delta && stepsTaken < 100) {
        step();
        timeAccumulator -= delta;
        stepsTaken++;
    }
}

void SpringPhysics::recordPoint() {
    history.push_back({ t, x, v, a, f, g, h, w, xe, err });

    // Prune history if it exceeds limits
    if (history.size() > maxHistoryPoints) {
        history.erase(history.begin(), history.begin() + (history.size() - maxHistoryPoints));
    }
}

float SpringPhysics::computeExactSolution(float tQuery) const {
    if (m <= 1e-6f || c <= 0.0f) return 0.0f;

    // For constant/zero excitation w = const, h = 0:
    float w0 = (wFunc.type == FunctionType::Constant) ? wFunc.amplitude : 0.0f;
    float y0 = x0 - w0;
    float v_init = v0;

    float omega0 = std::sqrt(c / m);
    float beta = k / (2.0f * m);
    float disc = beta * beta - omega0 * omega0;

    if (disc < -1e-7f) {
        // Underdamped
        float omega_d = std::sqrt(omega0 * omega0 - beta * beta);
        float C1 = y0;
        float C2 = (v_init + beta * y0) / omega_d;
        float expTerm = std::exp(-beta * tQuery);
        return w0 + expTerm * (C1 * std::cos(omega_d * tQuery) + C2 * std::sin(omega_d * tQuery));
    }
    else if (std::abs(disc) <= 1e-7f) {
        // Critically damped
        float C1 = y0;
        float C2 = v_init + beta * y0;
        float expTerm = std::exp(-beta * tQuery);
        return w0 + expTerm * (C1 + C2 * tQuery);
    }
    else {
        // Overdamped
        float gamma = std::sqrt(disc);
        float r1 = -beta + gamma;
        float r2 = -beta - gamma;
        float C1 = (v_init - r2 * y0) / (r1 - r2);
        float C2 = y0 - C1;
        return w0 + C1 * std::exp(r1 * tQuery) + C2 * std::exp(r2 * tQuery);
    }
}
