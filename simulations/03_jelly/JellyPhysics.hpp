#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <array>
#include <vector>

// 4x4x4 mass-spring system (Bezier cube control points) attached to a rigid,
// user-controlled frame and bounded by an axis-aligned box.
class JellyPhysics {
public:
    static constexpr int N = 4;              // control points per edge
    static constexpr int COUNT = N * N * N;  // 64 masses
    static constexpr float a = 1.0f;         // cube edge length

    struct Spring { int i, j; float l0; };   // l0 = rest length

    // --- Parameters (Krok 1.3, Krok 4) ---
    float m = 1.0f;            // mass of a single control point
    float c1 = 50.0f;          // stiffness of springs between masses
    float k = 0.5f;            // viscous damping
    float c2 = 20.0f;          // stiffness of corner <-> frame springs
    float disturbance = 3.0f;  // max magnitude of random initial velocities
    float mu = 0.5f;           // restitution: |v_n| after = mu * |v_n| before
    const glm::vec3 boxHalf{ 2.0f, 1.5f, 2.0f }; // bounding box half-extents (centered at origin)

    // --- Control frame (rigid cube of edge a) ---
    glm::vec3 framePos{ 0.0f };
    glm::quat frameRot{ 1.0f, 0.0f, 0.0f, 0.0f };

    // --- State ---
    std::array<glm::vec3, COUNT> p{}, v{};  // positions and velocities
    std::vector<Spring> springs;           // edge springs first, then face diagonals
    int edgeSpringCount = 0;
    float t = 0.0f;

    // Simulation runtime controls
    bool isRunning = true;
    float simTimeSpeed = 1.0f;
    float delta = 0.001f; // integration step (s)
    int maxSubStepsPerFrame = 200;

public:
    JellyPhysics();

    void reset();     // equilibrium state, frame back to origin
    void disturb();   // add random velocities to all masses
    void step();
    void update(float realDt);

    static int idx(int i, int j, int k) { return (i * N + j) * N + k; }
    // Index of the control point at frame corner n (bits of n = corner in x, y, z)
    static int cornerIndex(int n) { return idx((N - 1) * ((n >> 2) & 1), (N - 1) * ((n >> 1) & 1), (N - 1) * (n & 1)); }

    glm::vec3 restPoint(int i, int j, int k) const; // equilibrium position of P_ijk in the frame
    std::array<glm::vec3, 8> frameCorners() const;

private:
    float timeAccumulator = 0.0f;

    void buildSprings();
    void collide(glm::vec3& pos, glm::vec3& vel) const;
};
