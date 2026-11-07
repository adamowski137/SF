#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>
#include <string>

enum class CubeSimPreset {
    RegularPrecession = 0,
    NutationLoops = 1,
    FreeRigidBody = 2,
    PhysicalPendulum = 3,
    SleepingTop = 4
};

class CubePhysics {
public:
    // --- Cube Dimension & Density ---
    float a = 1.0f; // length of the side of the cube (m)
    float rho = 500.0f; // density (kg / m^3)

    // Derived physical properties (updated automatically)
    float mass = 500.0f; // M = rho * a^3 (kg)
    glm::vec3 r_cm{ 0.5f, 0.5f, 0.5f }; // center of mass in body coordinates
    glm::mat3 I{ 1.0f }; // 3x3 inertia tensor with respect to corner
    glm::mat3 invI{ 1.0f }; // inverse inertia tensor

    // --- Gravity ---
    bool gravityEnabled = true;
    float gValue = 9.81f; // m/s^2
    glm::vec3 gravityDir{ 0.0f, -1.0f, 0.0f }; // normalized gravity direction in world space

    // --- Optional Damping ---
    float dampingCoeff = 0.0f; // rotational air resistance torque: -c * W

    // --- Initial Condition Settings (Krok 1.3) ---
    float initThetaDeg = 30.0f;  // polar inclination of diagonal from vertical (degrees)
    float initPhiDeg = 0.0f;    // azimuth angle of tilt in horizontal plane (degrees)
    float initPsiDeg = 0.0f;    // rotation angle around diagonal (degrees)
    float initOmegaDiag = 5.0f;  // angular velocity around diagonal (rad/s)
    float initOmegaPerp = 0.0f;  // transverse angular velocity (rad/s, for nutation)

    // --- Simulation State ---
    float t = 0.0f;
    glm::quat Q{ 1.0f, 0.0f, 0.0f, 0.0f }; // orientation quaternion (body -> world)
    glm::vec3 W{ 0.0f, 0.0f, 0.0f };      // angular velocity in body frame (rad/s)

    // Simulation runtime controls
    bool isRunning = true;
    float simTimeSpeed = 1.0f;
    float delta = 0.001f; // RK4 time step size (s)
    int maxSubStepsPerFrame = 200;

    // --- Trajectory of the free diagonal end (Krok 1.1 & 1.3) ---
    int trajectoryLengthN = 1000; // max number of points to keep
    std::vector<glm::vec3> trajectory; // world positions of tip

public:
    CubePhysics();

    void reset();
    void step();
    void update(float realDt);

    // Update side length and density, recalculating inertia tensor
    void setSideLength(float newA);
    void setDimensions(float newA);
    void setDensity(float newRho);
    void updateInertiaTensor();

    // Orientation and angular velocity configuration
    void setInitialConditions(float thetaDeg, float phiDeg, float omegaDiag, float psiDeg = 0.0f, float omegaPerp = 0.0f);
    void applyOrientationFromDiagonal(float thetaDeg, float phiDeg, float psiDeg = 0.0f);
    void applyAngularVelocityAroundDiagonal(float omegaDiag, float omegaPerp = 0.0f);

    // Presets
    void loadPreset(CubeSimPreset preset);

    // Queries
    glm::vec3 getTipWorldPosition() const;
    glm::vec3 getDiagonalBody() const;
    glm::vec3 getDiagonalUnitBody() const;
    glm::vec3 getDiagonalWorld() const;

    void clearTrajectory();

private:
    float timeAccumulator = 0.0f;

    struct Derivative {
        glm::quat dQ;
        glm::vec3 dW;
    };

    Derivative evaluateDerivative(const glm::quat& qState, const glm::vec3& wState) const;
};
