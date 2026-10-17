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

        case FunctionType::BangBang_Ex6: {
            float x_opt = 0.0f, v_opt = 0.0f, a_opt = 0.0f, w_opt = 0.0f;
            SpringPhysics::getExercise6OptimalTrajectory(t, x_opt, v_opt, a_opt, w_opt, m, c, k);
            return w_opt;
        }

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

void SpringPhysics::setupExercise1() {
    m = 1.0f;
    c = 1.0f;
    k = 0.5f;
    x0 = 1.0f;
    v0 = 0.0f;
    delta = 0.01f;
    wFunc.type = FunctionType::Constant;
    wFunc.amplitude = 0.0f;
    hFunc.type = FunctionType::Constant;
    hFunc.amplitude = 0.0f;
    integrator = IntegratorType::RungeKutta4;
    reset();
}

std::vector<ConvergenceResult> SpringPhysics::runExercise1ConvergenceTest(
    IntegratorType method, const std::vector<float>& deltas, float testDuration)
{
    std::vector<ConvergenceResult> results;
    for (float dt_val : deltas) {
        SpringPhysics simCopy = *this;
        simCopy.integrator = method;
        simCopy.delta = dt_val;
        simCopy.reset();

        float maxErr = 0.0f;
        float sumSq = 0.0f;
        int count = 0;

        int totalSteps = static_cast<int>(testDuration / dt_val);
        for (int i = 0; i < totalSteps; i++) {
            simCopy.step();
            float currErr = std::abs(simCopy.x - simCopy.xe);
            if (currErr > maxErr) maxErr = currErr;
            sumSq += currErr * currErr;
            count++;
        }

        ConvergenceResult cr;
        cr.delta = dt_val;
        cr.maxError = maxErr;
        cr.rmsError = (count > 0) ? std::sqrt(sumSq / count) : 0.0f;
        results.push_back(cr);
    }
    return results;
}

void SpringPhysics::setupExercise2() {
    m = 1.0f;
    c = 1.0f;
    k = 1.0f;
    x0 = 1.0f;
    v0 = 0.0f;
    delta = 0.005f;
    wFunc.type = FunctionType::Constant;
    wFunc.amplitude = 0.0f;
    hFunc.type = FunctionType::Constant;
    hFunc.amplitude = 0.0f;
    integrator = IntegratorType::RungeKutta4;
    reset();
}

float SpringPhysics::runExercise2CriticalDampingSearch(float& theoreticalK, float maxT) {
    theoreticalK = 2.0f * std::sqrt(m * c);

    // Bisection search for k_kr
    // Condition: for k < k_kr, x(t) crosses zero (oscillates around 0)
    auto doesOscillate = [this, maxT](float testK) -> bool {
        SpringPhysics testSim = *this;
        testSim.k = testK;
        testSim.delta = 0.005f;
        testSim.integrator = IntegratorType::RungeKutta4;
        testSim.reset();

        int totalSteps = static_cast<int>(maxT / testSim.delta);
        for (int i = 0; i < totalSteps; i++) {
            testSim.step();
            if (testSim.x < -1e-4f) {
                return true; // Crossed zero
            }
        }
        return false;
    };

    float low = 0.0f;
    float high = theoreticalK * 1.5f;

    for (int iter = 0; iter < 25; iter++) {
        float mid = 0.5f * (low + high);
        if (doesOscillate(mid)) {
            low = mid;
        } else {
            high = mid;
        }
    }

    return 0.5f * (low + high);
}

void SpringPhysics::setupExercise3() {
    m = 1.0f;
    c = 1.0f;
    k = 1.5f;
    x0 = 0.0f;
    v0 = 0.0f;
    delta = 0.005f;
    wFunc.type = FunctionType::Step;
    wFunc.amplitude = 1.0f;
    wFunc.stepTime = 0.0f;
    hFunc.type = FunctionType::Constant;
    hFunc.amplitude = 0.0f;
    integrator = IntegratorType::RungeKutta4;
    reset();
}

std::vector<SettlingPoint> SpringPhysics::runExercise3SettlingSweep(
    float tolerance, float maxT, float kMin, float kMax, int steps)
{
    std::vector<SettlingPoint> results;
    float targetX = 1.0f;

    for (int i = 0; i <= steps; i++) {
        float testK = kMin + (kMax - kMin) * (static_cast<float>(i) / static_cast<float>(steps));
        SpringPhysics testSim = *this;
        testSim.k = testK;
        testSim.x0 = 0.0f;
        testSim.v0 = 0.0f;
        testSim.wFunc.type = FunctionType::Step;
        testSim.wFunc.amplitude = targetX;
        testSim.wFunc.stepTime = 0.0f;
        testSim.delta = 0.005f;
        testSim.integrator = IntegratorType::RungeKutta4;
        testSim.reset();

        int totalSteps = static_cast<int>(maxT / testSim.delta);
        std::vector<std::pair<float, float>> trajectory;
        trajectory.reserve(totalSteps);

        for (int s = 0; s < totalSteps; s++) {
            testSim.step();
            trajectory.emplace_back(testSim.t, testSim.x);
        }

        // Find settling time: time after which |x - targetX| remains <= tolerance
        float settleT = maxT;
        for (int idx = static_cast<int>(trajectory.size()) - 1; idx >= 0; idx--) {
            if (std::abs(trajectory[idx].second - targetX) > tolerance) {
                settleT = (idx < static_cast<int>(trajectory.size()) - 1) ? trajectory[idx + 1].first : maxT;
                break;
            }
        }

        results.push_back({ testK, settleT });
    }

    return results;
}

void SpringPhysics::setupExercise4() {
    m = 1.0f;
    c = 1.0f;
    k = 0.5f;
    x0 = 1.0f;
    v0 = 0.0f;
    delta = 0.005f;
    wFunc.type = FunctionType::Square;
    wFunc.amplitude = 1.0f;
    wFunc.omega = 0.5f; // square wave frequency
    wFunc.phi = 0.0f;
    hFunc.type = FunctionType::Constant;
    hFunc.amplitude = 0.0f;
    integrator = IntegratorType::RungeKutta4;
    reset();
}

float SpringPhysics::computeExercise4SettlingTime(float threshold) const {
    if (history.empty()) return -1.0f;

    // Search for T such that for all t > T in history, |w(t) - x(t)| < threshold
    float T = -1.0f;
    for (int i = static_cast<int>(history.size()) - 1; i >= 0; i--) {
        if (std::abs(history[i].w - history[i].x) >= threshold) {
            if (i + 1 < static_cast<int>(history.size())) {
                T = history[i + 1].t;
            }
            break;
        }
    }
    return T;
}

void SpringPhysics::setupExercise5() {
    m = 1.0f;
    c = 1.0f;
    k = 0.1f;
    x0 = 1.0f;
    v0 = 0.0f;
    delta = 0.005f;
    wFunc.type = FunctionType::Sine;
    wFunc.amplitude = 1.0f;
    wFunc.omega = 1.0f;
    wFunc.phi = 0.0f;
    hFunc.type = FunctionType::Constant;
    hFunc.amplitude = 0.0f;
    integrator = IntegratorType::RungeKutta4;
    reset();
}

std::vector<ResonanceCurve> SpringPhysics::computeExercise5ResonanceCurves(
    const std::vector<float>& kValues, float omegaMin, float omegaMax, int steps) const
{
    std::vector<ResonanceCurve> curves;
    float A0 = 1.0f;

    for (float kVal : kValues) {
        ResonanceCurve curve;
        curve.k = kVal;

        // Resonance peak: omega_r = sqrt(c/m - k^2 / (2m^2))
        float underRoot = (c / m) - (kVal * kVal) / (2.0f * m * m);
        if (underRoot > 0.0f) {
            curve.omega_r = std::sqrt(underRoot);
            float denom = kVal * std::sqrt((c / m) - (kVal * kVal) / (4.0f * m * m));
            curve.max_amplitude = (denom > 1e-6f) ? (c * A0 / denom) : 0.0f;
        } else {
            curve.omega_r = 0.0f;
            curve.max_amplitude = A0;
        }

        for (int i = 0; i <= steps; i++) {
            float wVal = omegaMin + (omegaMax - omegaMin) * (static_cast<float>(i) / static_cast<float>(steps));
            // Steady-state amplitude from equation:
            // A(omega) = c * A0 / sqrt((c - m * omega^2)^2 + (k * omega)^2)
            float realPart = c - m * wVal * wVal;
            float imagPart = kVal * wVal;
            float denom = std::sqrt(realPart * realPart + imagPart * imagPart);
            float amp = (denom > 1e-6f) ? (c * A0 / denom) : 0.0f;
            float phase = -std::atan2(imagPart, realPart);

            curve.points.push_back({ wVal, amp, phase });
        }
        curves.push_back(curve);
    }

    return curves;
}

void SpringPhysics::setupExercise6() {
    m = 1.0f;
    c = 1.0f;
    k = 0.1f;
    x0 = 0.0f;
    v0 = 0.0f;
    delta = 0.002f;
    wFunc.type = FunctionType::BangBang_Ex6;
    hFunc.type = FunctionType::Constant;
    hFunc.amplitude = 0.0f;
    integrator = IntegratorType::RungeKutta4;
    reset();
}

void SpringPhysics::getExercise6OptimalTrajectory(
    float tQuery, float& x_out, float& v_out, float& a_out, float& w_out, float mass, float stiff, float damp)
{
    // Pontryagin's Minimum Time Principle for dx = 1, x(0)=0, v(0)=0, x(T)=1, v(T)=0 with |a| <= 1:
    // a(t) = +1 for t in [0, 1)
    // a(t) = -1 for t in [1, 2)
    // a(t) = 0  for t >= 2
    // Minimum time T = 2.0s
    if (tQuery < 0.0f) {
        a_out = 0.0f;
        v_out = 0.0f;
        x_out = 0.0f;
    }
    else if (tQuery < 1.0f) {
        a_out = 1.0f;
        v_out = tQuery;
        x_out = 0.5f * tQuery * tQuery;
    }
    else if (tQuery < 2.0f) {
        a_out = -1.0f;
        float dt_1 = tQuery - 1.0f;
        v_out = 1.0f - dt_1;
        x_out = 0.5f + 1.0f * dt_1 - 0.5f * dt_1 * dt_1;
    }
    else {
        a_out = 0.0f;
        v_out = 0.0f;
        x_out = 1.0f;
    }

    // Required excitation: m * a + k * v + c * x = c * w => w = x + (m/c)*a + (k/c)*v
    if (stiff > 1e-6f) {
        w_out = x_out + (mass / stiff) * a_out + (damp / stiff) * v_out;
    } else {
        w_out = x_out;
    }
}
