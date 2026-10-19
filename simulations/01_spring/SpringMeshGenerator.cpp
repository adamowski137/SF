#include "SpringMeshGenerator.hpp"
#include "core/MeshGenerator.hpp"
#include <glm/glm.hpp>
#include <cmath>

void SpringMeshGenerator::init() {
    mountMesh = MeshGenerator::GetCylinderMesh(0.45f, 0.12f, 24, 2);
    buildRulerMesh();

    generateSpringVertices(getTopY(0.0f), getBottomY(0.0f), scratchVerts, scratchIndices);
    springMesh = Mesh::createMesh(scratchVerts, scratchIndices, { {3, GL_FLOAT}, {3, GL_FLOAT} });
}

void SpringMeshGenerator::updateSpring(float w_val, float x_val) {
    if (!springMesh) return;

    generateSpringVertices(getTopY(w_val), getBottomY(x_val), scratchVerts, scratchIndices);
    springMesh->updateMesh(scratchVerts, scratchIndices);
}

void SpringMeshGenerator::generateSpringVertices(
    float yTop, float yBottom, std::vector<float>& vertices, std::vector<unsigned int>& indices)
{
    const int totalSlices = params.numCoils * params.segmentsPerCoil;
    const int M = params.circleSegments;
    const float twoPi = 6.28318530718f;

    vertices.clear();
    indices.clear();

    struct PathNode {
        glm::vec3 pos;
        glm::vec3 t, n, b;
    };

    std::vector<PathNode> path(totalSlices + 1);

    for (int i = 0; i <= totalSlices; i++) {
        float s = static_cast<float>(i) / static_cast<float>(totalSlices);
        float y = yTop + s * (yBottom - yTop);
        float angle = twoPi * params.numCoils * s;

        float taper = 1.0f;
        if (s < 0.04f) taper = s / 0.04f;
        else if (s > 0.96f) taper = (1.0f - s) / 0.04f;

        float R = params.coilRadius * taper;
        path[i].pos = glm::vec3(R * std::cos(angle), y, R * std::sin(angle));
    }

    for (int i = 0; i <= totalSlices; i++) {
        glm::vec3 tangent = (i == 0) ? (path[1].pos - path[0].pos) :
                            (i == totalSlices) ? (path[totalSlices].pos - path[totalSlices - 1].pos) :
                            (path[i + 1].pos - path[i - 1].pos);

        float tLen = glm::length(tangent);
        path[i].t = (tLen > 1e-6f) ? (tangent / tLen) : glm::vec3(0.0f, -1.0f, 0.0f);

        float s = static_cast<float>(i) / static_cast<float>(totalSlices);
        float angle = twoPi * params.numCoils * s;
        glm::vec3 radial(std::cos(angle), 0.0f, std::sin(angle));

        glm::vec3 binormal = glm::cross(path[i].t, radial);
        float bLen = glm::length(binormal);
        path[i].b = (bLen > 1e-6f) ? (binormal / bLen) : glm::vec3(1.0f, 0.0f, 0.0f);

        path[i].n = glm::normalize(glm::cross(path[i].b, path[i].t));
    }

    const float rWire = params.wireRadius;
    vertices.reserve((totalSlices + 1) * M * 6 + 12);
    for (int i = 0; i <= totalSlices; i++) {
        const auto& node = path[i];
        for (int j = 0; j < M; j++) {
            float phi = twoPi * static_cast<float>(j) / static_cast<float>(M);
            glm::vec3 norm = std::cos(phi) * node.n + std::sin(phi) * node.b;
            glm::vec3 pos = node.pos + rWire * norm;

            vertices.push_back(pos.x);
            vertices.push_back(pos.y);
            vertices.push_back(pos.z);
            vertices.push_back(norm.x);
            vertices.push_back(norm.y);
            vertices.push_back(norm.z);
        }
    }

    indices.reserve(totalSlices * M * 6 + M * 6);
    for (int i = 0; i < totalSlices; i++) {
        int ring1 = i * M;
        int ring2 = (i + 1) * M;
        for (int j = 0; j < M; j++) {
            int next_j = (j + 1) % M;

            unsigned int i0 = ring1 + j;
            unsigned int i1 = ring1 + next_j;
            unsigned int i2 = ring2 + next_j;
            unsigned int i3 = ring2 + j;

            indices.push_back(i0);
            indices.push_back(i1);
            indices.push_back(i3);

            indices.push_back(i1);
            indices.push_back(i2);
            indices.push_back(i3);
        }
    }

    unsigned int topCenterIdx = static_cast<unsigned int>(vertices.size() / 6);
    vertices.push_back(path[0].pos.x);
    vertices.push_back(path[0].pos.y);
    vertices.push_back(path[0].pos.z);
    vertices.push_back(-path[0].t.x);
    vertices.push_back(-path[0].t.y);
    vertices.push_back(-path[0].t.z);

    for (int j = 0; j < M; j++) {
        int next_j = (j + 1) % M;
        indices.push_back(topCenterIdx);
        indices.push_back(next_j);
        indices.push_back(j);
    }

    unsigned int bottomCenterIdx = static_cast<unsigned int>(vertices.size() / 6);
    int lastRingStart = totalSlices * M;
    vertices.push_back(path[totalSlices].pos.x);
    vertices.push_back(path[totalSlices].pos.y);
    vertices.push_back(path[totalSlices].pos.z);
    vertices.push_back(path[totalSlices].t.x);
    vertices.push_back(path[totalSlices].t.y);
    vertices.push_back(path[totalSlices].t.z);

    for (int j = 0; j < M; j++) {
        int next_j = (j + 1) % M;
        indices.push_back(bottomCenterIdx);
        indices.push_back(lastRingStart + j);
        indices.push_back(lastRingStart + next_j);
    }
}

void SpringMeshGenerator::buildRulerMesh() {
    std::vector<float> verts;
    std::vector<unsigned int> idxs;

    float eqY = params.mountY - params.restLength;
    float rulerX = -0.8f;

    auto addVertex = [&](float x, float y, float z) {
        verts.push_back(x);
        verts.push_back(y);
        verts.push_back(z);
        verts.push_back(1.0f);
        verts.push_back(0.0f);
        verts.push_back(0.0f);
    };

    for (int tick = -4; tick <= 4; tick++) {
        float y = eqY + static_cast<float>(tick);
        float tickLen = (tick == 0) ? 0.25f : ((tick % 2 == 0) ? 0.18f : 0.10f);
        float z0 = -tickLen * 0.5f;
        float z1 = tickLen * 0.5f;
        float dy = 0.015f;

        unsigned int base = static_cast<unsigned int>(verts.size() / 6);
        addVertex(rulerX, y - dy, z0);
        addVertex(rulerX, y - dy, z1);
        addVertex(rulerX, y + dy, z1);
        addVertex(rulerX, y + dy, z0);

        idxs.push_back(base);
        idxs.push_back(base + 1);
        idxs.push_back(base + 2);
        idxs.push_back(base);
        idxs.push_back(base + 2);
        idxs.push_back(base + 3);
    }

    rulerMesh = Mesh::createMesh(verts, idxs, { {3, GL_FLOAT}, {3, GL_FLOAT} });
}
