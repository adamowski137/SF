#include "CubePhysics.hpp"
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

CubePhysics::CubePhysics() {
    updateInertiaTensor();
    reset();
}

void CubePhysics::setSideLength(float newA) {
    a = std::max(0.05f, newA);
    updateInertiaTensor();
}

void CubePhysics::setDimensions(float newA) {
    setSideLength(newA);
}

void CubePhysics::setDensity(float newRho) {
    rho = std::max(0.1f, newRho);
    updateInertiaTensor();
}

void CubePhysics::updateInertiaTensor() {
    mass = rho * a * a * a;
    float diagLen = a * std::sqrt(3.0f);
    r_cm = glm::vec3(0.0f, diagLen * 0.5f, 0.0f);

    float I_perp = (11.0f / 12.0f) * mass * a * a;
    float I_diag = (1.0f / 6.0f) * mass * a * a;

    I = glm::mat3(
        I_perp, 0.0f,   0.0f,
        0.0f,   I_diag, 0.0f,
        0.0f,   0.0f,   I_perp
    );

    invI = glm::mat3(
        1.0f / I_perp, 0.0f,          0.0f,
        0.0f,          1.0f / I_diag, 0.0f,
        0.0f,          0.0f,          1.0f / I_perp
    );
}

glm::vec3 CubePhysics::getDiagonalBody() const {
    return glm::vec3(0.0f, a * std::sqrt(3.0f), 0.0f);
}

glm::vec3 CubePhysics::getDiagonalUnitBody() const {
    return glm::vec3(0.0f, 1.0f, 0.0f);
}

glm::vec3 CubePhysics::getDiagonalWorld() const {
    return glm::mat3_cast(Q) * getDiagonalBody();
}

glm::vec3 CubePhysics::getTipWorldPosition() const {
    return getDiagonalWorld();
}

void CubePhysics::applyOrientationFromDiagonal(float thetaDeg, float phiDeg, float psiDeg) {
    float thetaRad = thetaDeg * static_cast<float>(M_PI) / 180.0f;
    float phiRad = phiDeg * static_cast<float>(M_PI) / 180.0f;
    float psiRad = psiDeg * static_cast<float>(M_PI) / 180.0f;

    // In the local frame, the cube diagonal already lies along +Y (0, 1, 0).
    // Step 1: Spin around the Y axis (diagonal) by psi
    glm::quat Q_spin = glm::angleAxis(psiRad, glm::vec3(0.0f, 1.0f, 0.0f));

    // Step 2: Tilt by theta in azimuth direction phi
    glm::vec3 tiltAxis(-std::sin(phiRad), 0.0f, std::cos(phiRad));
    glm::quat Q_tilt = glm::angleAxis(thetaRad, glm::normalize(tiltAxis));

    Q = glm::normalize(Q_tilt * Q_spin);
}

void CubePhysics::applyAngularVelocityAroundDiagonal(float omegaDiag, float omegaPerp) {
    // In the local frame, the Y axis is the cube diagonal and X/Z are perpendicular principal axes
    W = glm::vec3(omegaPerp, omegaDiag, 0.0f);
}

void CubePhysics::setInitialConditions(float thetaDeg, float phiDeg, float omegaDiag, float psiDeg, float omegaPerp) {
    initThetaDeg = thetaDeg;
    initPhiDeg = phiDeg;
    initOmegaDiag = omegaDiag;
    initPsiDeg = psiDeg;
    initOmegaPerp = omegaPerp;
    reset();
}

void CubePhysics::reset() {
    t = 0.0f;
    timeAccumulator = 0.0f;

    applyOrientationFromDiagonal(initThetaDeg, initPhiDeg, initPsiDeg);
    applyAngularVelocityAroundDiagonal(initOmegaDiag, initOmegaPerp);

    clearTrajectory();
}

void CubePhysics::clearTrajectory() {
    trajectory.clear();
    trajectory.push_back(getTipWorldPosition());
}

CubePhysics::Derivative CubePhysics::evaluateDerivative(const glm::quat& qState, const glm::vec3& wState) const {
    glm::quat qNorm = glm::normalize(qState);
    glm::mat3 R = glm::mat3_cast(qNorm);

    // Gravity force and torque in body frame:
    // In world frame: F_g = mass * (gValue * gravityDir)
    // In body frame: g_body = R^T * (gValue * gravityDir)
    glm::vec3 gWorld = gravityDir * gValue;
    glm::vec3 torque(0.0f);

    if (gravityEnabled) {
        glm::vec3 g_body = glm::transpose(R) * gWorld;
        torque = glm::cross(r_cm, mass * g_body);
    }

    // Euler's rotational equation in body frame:
    // I * dW = N + (I * W) x W
    // dW = invI * (N + cross(I * W, W))
    glm::vec3 Iw = I * wState;
    glm::vec3 gyroscopic = glm::cross(Iw, wState);
    glm::vec3 dW = invI * (torque + gyroscopic);

    // Quaternion kinematic differential equation:
    // dQ = 0.5 * Q * (0, W)
    glm::quat w_quat(0.0f, wState.x, wState.y, wState.z);
    glm::quat dQ = 0.5f * (qNorm * w_quat);

    return { dQ, dW };
}

void CubePhysics::step() {
    if (delta <= 1e-7f) return;

    float h = delta;

    // Runge-Kutta 4 simultaneous integration for Q and W:
    Derivative k1 = evaluateDerivative(Q, W);

    glm::quat Q2 = Q + 0.5f * h * k1.dQ;
    glm::vec3 W2 = W + 0.5f * h * k1.dW;
    Derivative k2 = evaluateDerivative(Q2, W2);

    glm::quat Q3 = Q + 0.5f * h * k2.dQ;
    glm::vec3 W3 = W + 0.5f * h * k2.dW;
    Derivative k3 = evaluateDerivative(Q3, W3);

    glm::quat Q4 = Q + h * k3.dQ;
    glm::vec3 W4 = W + h * k3.dW;
    Derivative k4 = evaluateDerivative(Q4, W4);

    W += (h / 6.0f) * (k1.dW + 2.0f * k2.dW + 2.0f * k3.dW + k4.dW);
    Q += (h / 6.0f) * (k1.dQ + 2.0f * k2.dQ + 2.0f * k3.dQ + k4.dQ);

    Q = glm::normalize(Q);

    t += h;

    glm::vec3 currentTip = getTipWorldPosition();
    trajectory.push_back(currentTip);
    if (static_cast<int>(trajectory.size()) > trajectoryLengthN) {
        size_t excess = trajectory.size() - trajectoryLengthN;
        trajectory.erase(trajectory.begin(), trajectory.begin() + excess);
    }
}

void CubePhysics::update(float realDt) {
    if (!isRunning) return;

    float effectiveDt = realDt * simTimeSpeed;
    if (effectiveDt <= 0.0f) return;

    timeAccumulator += effectiveDt;

    int stepsTaken = 0;
    while (timeAccumulator >= delta && stepsTaken < maxSubStepsPerFrame) {
        step();
        timeAccumulator -= delta;
        stepsTaken++;
    }
}

void CubePhysics::loadPreset(CubeSimPreset preset) {
    switch (preset) {
        case CubeSimPreset::RegularPrecession:
            gravityEnabled = true;
            gValue = 9.81f;
            initThetaDeg = 30.0f;
            initPhiDeg = 0.0f;
            initPsiDeg = 0.0f;
            initOmegaDiag = 5.0f;
            initOmegaPerp = 0.0f;
            break;

        case CubeSimPreset::PhysicalPendulum:
            gravityEnabled = true;
            gValue = 9.81f;
            initThetaDeg = 60.0f;
            initPhiDeg = 0.0f;
            initPsiDeg = 0.0f;
            initOmegaDiag = 0.0f; // no spin, purely swinging under gravity
            initOmegaPerp = 0.0f;
            break;

        case CubeSimPreset::SleepingTop:
            gravityEnabled = true;
            gValue = 9.81f;
            initThetaDeg = 0.1f; // almost straight upright
            initPhiDeg = 0.0f;
            initPsiDeg = 0.0f;
            initOmegaDiag = 7.0f; // vertical spin
            initOmegaPerp = 0.0f;
            break;
    }
    reset();
}
