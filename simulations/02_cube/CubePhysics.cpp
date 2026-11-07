#include "CubePhysics.hpp"
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace {
    glm::quat createRotationBetweenVectors(const glm::vec3& u, const glm::vec3& v) {
        glm::vec3 u_norm = glm::normalize(u);
        glm::vec3 v_norm = glm::normalize(v);
        float cosTheta = glm::clamp(glm::dot(u_norm, v_norm), -1.0f, 1.0f);

        if (cosTheta > 0.999999f) {
            return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        }
        if (cosTheta < -0.999999f) {
            // 180 degree rotation: pick an orthogonal axis
            glm::vec3 ortho = glm::cross(glm::vec3(1.0f, 0.0f, 0.0f), u_norm);
            if (glm::dot(ortho, ortho) < 0.01f) {
                ortho = glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), u_norm);
            }
            return glm::angleAxis(static_cast<float>(M_PI), glm::normalize(ortho));
        }

        glm::vec3 axis = glm::cross(u_norm, v_norm);
        float angle = std::acos(cosTheta);
        return glm::angleAxis(angle, glm::normalize(axis));
    }
}

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
    r_cm = glm::vec3(a * 0.5f);

    float diag = (2.0f / 3.0f) * mass * a * a;
    float offDiag = -0.25f * mass * a * a;

    // GLM mat3 is column-major: mat3(col0, col1, col2)
    // For a uniform cube anchored at corner (0,0,0):
    // Ixx = Iyy = Izz = (2/3)*mass*a^2, Ixy = Ixz = Iyz = -0.25*mass*a^2
    I = glm::mat3(
        diag, offDiag, offDiag,
        offDiag, diag, offDiag,
        offDiag, offDiag, diag
    );

    invI = glm::inverse(I);
}

glm::vec3 CubePhysics::getDiagonalBody() const {
    return glm::vec3(a);
}

glm::vec3 CubePhysics::getDiagonalUnitBody() const {
    return glm::normalize(glm::vec3(1.0f));
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

    glm::vec3 d_body = getDiagonalUnitBody();
    glm::vec3 upWorld = -glm::normalize(gravityDir); // normally (0, 1, 0)

    // Step 1: Align body diagonal with upWorld
    glm::quat Q_align = createRotationBetweenVectors(d_body, upWorld);

    // Step 2: Spin around upWorld / diagonal
    glm::quat Q_spin = glm::angleAxis(psiRad, upWorld);

    // Step 3: Tilt by theta in direction phi
    // If upWorld = (0, 1, 0): tiltAxis is (-sin(phi), 0, cos(phi))
    glm::vec3 horizRef(1.0f, 0.0f, 0.0f);
    if (std::abs(glm::dot(horizRef, upWorld)) > 0.9f) {
        horizRef = glm::vec3(0.0f, 0.0f, 1.0f);
    }
    glm::vec3 horizX = glm::normalize(horizRef - upWorld * glm::dot(horizRef, upWorld));
    glm::vec3 horizZ = glm::normalize(glm::cross(upWorld, horizX));

    glm::vec3 tiltAxis = -std::sin(phiRad) * horizX + std::cos(phiRad) * horizZ;
    glm::quat Q_tilt = glm::angleAxis(thetaRad, glm::normalize(tiltAxis));

    Q = glm::normalize(Q_tilt * Q_spin * Q_align);
}

void CubePhysics::applyAngularVelocityAroundDiagonal(float omegaDiag, float omegaPerp) {
    glm::vec3 d_hat = getDiagonalUnitBody();
    glm::vec3 w_spin = omegaDiag * d_hat;

    glm::vec3 w_trans(0.0f);
    if (std::abs(omegaPerp) > 1e-6f) {
        glm::vec3 ref(1.0f, 0.0f, 0.0f);
        if (std::abs(glm::dot(ref, d_hat)) > 0.9f) {
            ref = glm::vec3(0.0f, 1.0f, 0.0f);
        }
        glm::vec3 perpAxis = glm::normalize(glm::cross(d_hat, ref));
        w_trans = omegaPerp * perpAxis;
    }

    W = w_spin + w_trans;
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

    // Normalize quaternion Q_{i+1} as required by lab specification
    Q = glm::normalize(Q);

    t += h;

    // Record trajectory point
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

        case CubeSimPreset::NutationLoops:
            gravityEnabled = true;
            gValue = 9.81f;
            initThetaDeg = 35.0f;
            initPhiDeg = 0.0f;
            initPsiDeg = 0.0f;
            initOmegaDiag = 3.5f;
            initOmegaPerp = 0.3f; // perturbation causing looping nutation
            break;

        case CubeSimPreset::FreeRigidBody:
            gravityEnabled = false;
            initThetaDeg = 45.0f;
            initPhiDeg = 20.0f;
            initPsiDeg = 0.0f;
            initOmegaDiag = 2.5f;
            initOmegaPerp = 1.0f; // polhode / free torque-free rotation
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
