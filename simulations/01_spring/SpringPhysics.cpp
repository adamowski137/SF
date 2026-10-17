#include "SpringPhysics.hpp"
#include <algorithm>
#include <cmath>

float SignalFunction::evaluate(float t, float m, float c, float k) const {
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

void SpringPhysics::reset() {
    t = 0.0f;
    x = x0;
    v = v0;
    hasPrev = false;

    w = wFunc.evaluate(t, m, c, k);
    h = hFunc.evaluate(t, m, c, k);
    f = c * (w - x);
    g = -k * v;
    a = (m > 1e-6f) ? ((f + g + h) / m) : 0.0f;

    xe = computeExactSolution(t);
    err = x - xe;

    maxAbsoluteError = 0.0f;
    sumSquaredError = 0.0f;
    errorCount = 0;

    history.clear();
    history.reserve(2000);
    recordPoint();
}

float SpringPhysics::computeAcceleration(float curX, float curV, float curT) const {
    if (m <= 1e-6f) return 0.0f;
    float curW = wFunc.evaluate(curT, m, c, k);
    float curH = hFunc.evaluate(curT, m, c, k);
    float curF = c * (curW - curX);
    float curG = -k * curV;
    return (curF + curG + curH) / m;
}

void SpringPhysics::step() {
    if (delta <= 1e-6f) return;

    float dt = delta;
    float nextX = x;
    float nextV = v;

    switch (integrator) {
        case IntegratorType::Euler: {
            float a_cur = computeAcceleration(x, v, t);
            nextX = x + dt * v;
            nextV = v + dt * a_cur;
            break;
        }

        case IntegratorType::SemiImplicitEuler: {
            float a_cur = computeAcceleration(x, v, t);
            nextV = v + dt * a_cur;
            nextX = x + dt * nextV;
            break;
        }

        case IntegratorType::CentralDifference: {
            // Central Difference approximation from PDF:
            // x_tt = (x(t+dt) - 2x(t) + x(t-dt)) / dt^2
            // x_t  = (x(t+dt) - x(t-dt)) / (2dt)
            // Substitute into m*x_tt + k*x_t + c*x = c*w + h
            float curW = wFunc.evaluate(t, m, c, k);
            float curH = hFunc.evaluate(t, m, c, k);
            if (!hasPrev) {
                // Initialize previous step x(t-dt) using Taylor expansion
                float a0 = computeAcceleration(x, v, t);
                x_prev = x - dt * v + 0.5f * dt * dt * a0;
                hasPrev = true;
            }

            float denom = m + 0.5f * k * dt;
            if (std::abs(denom) > 1e-8f) {
                float numer = 2.0f * m * x - (m - 0.5f * k * dt) * x_prev + dt * dt * (c * (curW - x) + curH);
                nextX = numer / denom;
                nextV = (nextX - x_prev) / (2.0f * dt);
                x_prev = x;
            } else {
                nextX = x + dt * v;
                nextV = v;
            }
            break;
        }

        case IntegratorType::RungeKutta4: {
            auto dydt = [this](float s_x, float s_v, float s_t) -> std::pair<float, float> {
                float acc = computeAcceleration(s_x, s_v, s_t);
                return { s_v, acc };
            };

            auto [k1_x, k1_v] = dydt(x, v, t);
            auto [k2_x, k2_v] = dydt(x + 0.5f * dt * k1_x, v + 0.5f * dt * k1_v, t + 0.5f * dt);
            auto [k3_x, k3_v] = dydt(x + 0.5f * dt * k2_x, v + 0.5f * dt * k2_v, t + 0.5f * dt);
            auto [k4_x, k4_v] = dydt(x + dt * k3_x, v + dt * k3_v, t + dt);

            nextX = x + (dt / 6.0f) * (k1_x + 2.0f * k2_x + 2.0f * k3_x + k4_x);
            nextV = v + (dt / 6.0f) * (k1_v + 2.0f * k2_v + 2.0f * k3_v + k4_v);
            break;
        }
    }

    t += dt;
    x = nextX;
    v = nextV;

    w = wFunc.evaluate(t, m, c, k);
    h = hFunc.evaluate(t, m, c, k);
    f = c * (w - x);
    g = -k * v;
    a = (m > 1e-6f) ? ((f + g + h) / m) : 0.0f;

    xe = computeExactSolution(t);
    err = x - xe;

    float absErr = std::abs(err);
    if (absErr > maxAbsoluteError) maxAbsoluteError = absErr;
    sumSquaredError += err * err;
    errorCount++;

    recordPoint();
}

void SpringPhysics::update(float realDt) {
    if (!isRunning) return;

    float effectiveDt = realDt * simTimeSpeed;
    if (effectiveDt <= 0.0f) return;

    // Fixed timestep accumulation
    int stepsToTake = static_cast<int>(std::floor(effectiveDt / delta));
    // Cap steps per frame to avoid freezing if delta is very small
    stepsToTake = std::clamp(stepsToTake, 1, 100);

    for (int i = 0; i < stepsToTake; i++) {
        step();
    }
}

void SpringPhysics::recordPoint() {
    SimPoint pt;
    pt.t = t;
    pt.x = x;
    pt.v = v;
    pt.a = a;
    pt.f = f;
    pt.g = g;
    pt.h = h;
    pt.w = w;
    pt.xe = xe;
    pt.err = err;
    history.push_back(pt);

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

float SpringPhysics::computeExactVelocity(float tQuery) const {
    if (m <= 1e-6f || c <= 0.0f) return 0.0f;

    float w0 = (wFunc.type == FunctionType::Constant) ? wFunc.amplitude : 0.0f;
    float y0 = x0 - w0;
    float v_init = v0;

    float omega0 = std::sqrt(c / m);
    float beta = k / (2.0f * m);
    float disc = beta * beta - omega0 * omega0;

    if (disc < -1e-7f) {
        float omega_d = std::sqrt(omega0 * omega0 - beta * beta);
        float C1 = y0;
        float C2 = (v_init + beta * y0) / omega_d;
        float expTerm = std::exp(-beta * tQuery);
        float dCos = std::cos(omega_d * tQuery);
        float dSin = std::sin(omega_d * tQuery);
        return expTerm * ((-beta * C1 + omega_d * C2) * dCos + (-beta * C2 - omega_d * C1) * dSin);
    }
    else if (std::abs(disc) <= 1e-7f) {
        float C1 = y0;
        float C2 = v_init + beta * y0;
        float expTerm = std::exp(-beta * tQuery);
        return expTerm * (C2 - beta * (C1 + C2 * tQuery));
    }
    else {
        float gamma = std::sqrt(disc);
        float r1 = -beta + gamma;
        float r2 = -beta - gamma;
        float C1 = (v_init - r2 * y0) / (r1 - r2);
        float C2 = y0 - C1;
        return C1 * r1 * std::exp(r1 * tQuery) + C2 * r2 * std::exp(r2 * tQuery);
    }
}

