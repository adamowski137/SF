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
    // Exercise 6: Optimal bang-bang control computes w(t) from the
    // time-optimal trajectory x*(t) via the inverted equation of motion:
    //   w(t) = x(t) + (k/c)*v(t) + (m/c)*a(t)
    // where x, v, a come from the analytical bang-bang trajectory.
    if (wFunc.type == FunctionType::OptimalBangBang) {
        if (t < 0.0f) {
            w = 0.0f;
        } else if (t < 1.0f) {
            float x_opt = 0.5f * t * t;
            float v_opt = t;
            float a_opt = 1.0f;
            w = x_opt + (k / c) * v_opt + (m / c) * a_opt;
        } else if (t <= 2.0f) {
            float x_opt = -0.5f * t * t + 2.0f * t - 1.0f;
            float v_opt = 2.0f - t;
            float a_opt = -1.0f;
            w = x_opt + (k / c) * v_opt + (m / c) * a_opt;
        } else {
            w = 1.0f; // Hold at x(T) = 1
        }
    } else {
        w = wFunc.evaluate(t);
    }
    h = hFunc.evaluate(t);
    f = c * (w - x);
    g = -k * v;
    a = (m > 1e-6f) ? ((f + g + h) / m) : 0.0f;
}

void SpringPhysics::reset() {
    t = 0.0f;
    x = x0;
    v = v0;
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
    history.push_back({ t, x, v, a, f, g, h, w });

    // Prune history if it exceeds limits
    if (history.size() > maxHistoryPoints) {
        history.erase(history.begin(), history.begin() + (history.size() - maxHistoryPoints));
    }
}
