#include "JellyPhysics.hpp"
#include <glm/gtc/random.hpp>
#include <cmath>

JellyPhysics::JellyPhysics() {
    buildSprings();
    reset();
}

glm::vec3 JellyPhysics::restPoint(int i, int j, int k) const {
    // P_ijk = a * (i/3, j/3, k/3), shifted so the cube is centered in the frame
    glm::vec3 local = a * (glm::vec3(i, j, k) / float(N - 1) - 0.5f);
    return framePos + frameRot * local;
}

std::array<glm::vec3, 8> JellyPhysics::frameCorners() const {
    std::array<glm::vec3, 8> c;
    for (int n = 0; n < 8; ++n)
        c[n] = restPoint((N - 1) * ((n >> 2) & 1), (N - 1) * ((n >> 1) & 1), (N - 1) * (n & 1));
    return c;
}

void JellyPhysics::buildSprings() {
    // Neighbours within one small cell (Rys. 1): edges (1 non-zero offset) and face diagonals (2 non-zero offsets)
    springs.clear();
    for (int pass = 1; pass <= 2; ++pass) {
        for (int i = 0; i < N; ++i) for (int j = 0; j < N; ++j) for (int k = 0; k < N; ++k)
            for (int di = -1; di <= 1; ++di) for (int dj = -1; dj <= 1; ++dj) for (int dk = -1; dk <= 1; ++dk) {
                if ((di != 0) + (dj != 0) + (dk != 0) != pass) continue;
                // take each pair once: offset must be lexicographically positive
                if (di < 0 || (di == 0 && (dj < 0 || (dj == 0 && dk < 0)))) continue;
                int ni = i + di, nj = j + dj, nk = k + dk;
                if (ni >= N || nj < 0 || nj >= N || nk < 0 || nk >= N) continue;
                float l0 = a / (N - 1) * std::sqrt(float(pass));
                springs.push_back({ idx(i, j, k), idx(ni, nj, nk), l0 });
            }
        if (pass == 1) edgeSpringCount = static_cast<int>(springs.size());
    }
}

void JellyPhysics::reset() {
    t = 0.0f;
    timeAccumulator = 0.0f;
    framePos = glm::vec3(0.0f);
    frameRot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

    for (int i = 0; i < N; ++i) for (int j = 0; j < N; ++j) for (int k = 0; k < N; ++k) {
        p[idx(i, j, k)] = restPoint(i, j, k);
        v[idx(i, j, k)] = glm::vec3(0.0f);
    }
}

void JellyPhysics::disturb() {
    for (auto& vel : v)
        vel += glm::ballRand(disturbance); // uniform in a ball => |v| <= disturbance
}

void JellyPhysics::step() {
    std::array<glm::vec3, COUNT> f;
    f.fill(glm::vec3(0.0f));

    // Mass-mass springs: f = -k * l' - c1 * l, acting along the spring (l = |P1 - P2| - l0)
    for (const auto& s : springs) {
        glm::vec3 d = p[s.i] - p[s.j];
        float len = glm::length(d);
        if (len < 1e-6f) continue;
        glm::vec3 dir = d / len;
        float l = len - s.l0;
        float ldot = glm::dot(v[s.i] - v[s.j], dir); // rate of elongation
        glm::vec3 F = (-k * ldot - c1 * l) * dir;
        f[s.i] += F;
        f[s.j] -= F;
    }

    // Corner-frame springs: rest length 0, stiffness c2 (frame is kinematic, its velocity is ignored)
    auto corners = frameCorners();
    for (int n = 0; n < 8; ++n) {
        int i = cornerIndex(n);
        glm::vec3 d = p[i] - corners[n];
        float len = glm::length(d);
        if (len < 1e-6f) continue;
        glm::vec3 dir = d / len;
        f[i] += (-k * glm::dot(v[i], dir) - c2 * len) * dir;
    }

    // Semi-implicit Euler, then resolve collisions with the bounding box
    for (int i = 0; i < COUNT; ++i) {
        v[i] += (f[i] / m) * delta;
        p[i] += v[i] * delta;
        collide(p[i], v[i]);
    }

    t += delta;
}

void JellyPhysics::collide(glm::vec3& pos, glm::vec3& vel) const {
    // Chosen model: v_after = (-mu * vx, vy, vz) - only the normal component is damped (mu = 0 sticks to the wall).
    // Each axis is resolved independently; the loop handles repeated reflections (overshoot larger than the box).
    for (int ax = 0; ax < 3; ++ax) {
        for (int it = 0; it < 16 && std::abs(pos[ax]) > boxHalf[ax]; ++it) {
            float wall = pos[ax] > 0.0f ? boxHalf[ax] : -boxHalf[ax];
            pos[ax] = wall - mu * (pos[ax] - wall); // mirror the overshoot, scaled by mu
            vel[ax] = -mu * vel[ax];
        }
        pos[ax] = glm::clamp(pos[ax], -boxHalf[ax], boxHalf[ax]); // guard if iteration limit was hit
    }
}

void JellyPhysics::update(float realDt) {
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
