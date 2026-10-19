#include "SpringMeshGenerator.hpp"
#include "core/MeshGenerator.hpp"
#include <glm/glm.hpp>
#include <cmath>

#define MOUNT_CYLINDER_RADIUS             0.45f
#define MOUNT_CYLINDER_HEIGHT             0.12f
#define MOUNT_CYLINDER_SECTOR_COUNT       24
#define MOUNT_CYLINDER_STACK_COUNT        2

#define INITIAL_DISPLACEMENT_W            0.0f
#define INITIAL_DISPLACEMENT_X            0.0f

#define VERTEX_POS_COMPONENTS             3
#define VERTEX_NORMAL_COMPONENTS          3
#define FLOATS_PER_VERTEX                 (VERTEX_POS_COMPONENTS + VERTEX_NORMAL_COMPONENTS)

#define SPRING_TWO_PI                     6.28318530718f
#define SPRING_FULL_TAPER                 1.0f
#define SPRING_TAPER_START_THRESHOLD      0.04f
#define SPRING_TAPER_END_THRESHOLD        (SPRING_FULL_TAPER - SPRING_TAPER_START_THRESHOLD)
#define SPRING_VECTOR_EPSILON             1e-6f
#define FALLBACK_TANGENT                  glm::vec3(0.0f, -1.0f, 0.0f)
#define FALLBACK_BINORMAL                 glm::vec3(1.0f, 0.0f, 0.0f)

#define ENDCAP_COUNT                      2
#define ENDCAP_VERTICES_FLOATS            (ENDCAP_COUNT * FLOATS_PER_VERTEX)
#define INDICES_PER_TRIANGLE              3
#define TRIANGLES_PER_QUAD                2
#define INDICES_PER_QUAD                  (TRIANGLES_PER_QUAD * INDICES_PER_TRIANGLE)
#define ENDCAP_INDICES_PER_SEGMENT        (ENDCAP_COUNT * INDICES_PER_TRIANGLE)

#define RULER_POSITION_X                  -0.8f
#define RULER_NORMAL_X                    1.0f
#define RULER_NORMAL_Y                    0.0f
#define RULER_NORMAL_Z                    0.0f
#define RULER_TICK_MIN                    -4
#define RULER_TICK_MAX                    4
#define RULER_CENTER_TICK_LENGTH          0.25f
#define RULER_MAJOR_TICK_LENGTH           0.18f
#define RULER_MINOR_TICK_LENGTH           0.10f
#define RULER_TICK_HALF_THICKNESS         0.015f
#define RULER_HALF_LENGTH_FACTOR          0.5f
#define RULER_MAJOR_TICK_INTERVAL         2

void SpringMeshGenerator::init() {
    mountMesh = MeshGenerator::GetCylinderMesh(
        MOUNT_CYLINDER_RADIUS,
        MOUNT_CYLINDER_HEIGHT,
        MOUNT_CYLINDER_SECTOR_COUNT,
        MOUNT_CYLINDER_STACK_COUNT);
    buildRulerMesh();

    generateSpringVertices(getTopY(INITIAL_DISPLACEMENT_W), getBottomY(INITIAL_DISPLACEMENT_X), scratchVerts, scratchIndices);
    springMesh = Mesh::createMesh(scratchVerts, scratchIndices, { {VERTEX_POS_COMPONENTS, GL_FLOAT}, {VERTEX_NORMAL_COMPONENTS, GL_FLOAT} });
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
    const float twoPi = SPRING_TWO_PI;

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

        float taper = SPRING_FULL_TAPER;
        if (s < SPRING_TAPER_START_THRESHOLD) {
            taper = s / SPRING_TAPER_START_THRESHOLD;
        } else if (s > SPRING_TAPER_END_THRESHOLD) {
            taper = (SPRING_FULL_TAPER - s) / SPRING_TAPER_START_THRESHOLD;
        }

        float R = params.coilRadius * taper;
        path[i].pos = glm::vec3(R * std::cos(angle), y, R * std::sin(angle));
    }

    for (int i = 0; i <= totalSlices; i++) {
        glm::vec3 tangent = (i == 0) ? (path[1].pos - path[0].pos) :
                            (i == totalSlices) ? (path[totalSlices].pos - path[totalSlices - 1].pos) :
                            (path[i + 1].pos - path[i - 1].pos);

        float tLen = glm::length(tangent);
        path[i].t = (tLen > SPRING_VECTOR_EPSILON) ? (tangent / tLen) : FALLBACK_TANGENT;

        float s = static_cast<float>(i) / static_cast<float>(totalSlices);
        float angle = twoPi * params.numCoils * s;
        glm::vec3 radial(std::cos(angle), 0.0f, std::sin(angle));

        glm::vec3 binormal = glm::cross(path[i].t, radial);
        float bLen = glm::length(binormal);
        path[i].b = (bLen > SPRING_VECTOR_EPSILON) ? (binormal / bLen) : FALLBACK_BINORMAL;

        path[i].n = glm::normalize(glm::cross(path[i].b, path[i].t));
    }

    const float rWire = params.wireRadius;
    vertices.reserve((totalSlices + 1) * M * FLOATS_PER_VERTEX + ENDCAP_VERTICES_FLOATS);
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

    indices.reserve(totalSlices * M * INDICES_PER_QUAD + M * ENDCAP_INDICES_PER_SEGMENT);
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

    unsigned int topCenterIdx = static_cast<unsigned int>(vertices.size() / FLOATS_PER_VERTEX);
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

    unsigned int bottomCenterIdx = static_cast<unsigned int>(vertices.size() / FLOATS_PER_VERTEX);
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
    float rulerX = RULER_POSITION_X;

    auto addVertex = [&](float x, float y, float z) {
        verts.push_back(x);
        verts.push_back(y);
        verts.push_back(z);
        verts.push_back(RULER_NORMAL_X);
        verts.push_back(RULER_NORMAL_Y);
        verts.push_back(RULER_NORMAL_Z);
    };

    for (int tick = RULER_TICK_MIN; tick <= RULER_TICK_MAX; tick++) {
        float y = eqY + static_cast<float>(tick);
        float tickLen = (tick == 0) ? RULER_CENTER_TICK_LENGTH :
                        ((tick % RULER_MAJOR_TICK_INTERVAL == 0) ? RULER_MAJOR_TICK_LENGTH : RULER_MINOR_TICK_LENGTH);
        float z0 = -tickLen * RULER_HALF_LENGTH_FACTOR;
        float z1 = tickLen * RULER_HALF_LENGTH_FACTOR;
        float dy = RULER_TICK_HALF_THICKNESS;

        unsigned int base = static_cast<unsigned int>(verts.size() / FLOATS_PER_VERTEX);
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

    rulerMesh = Mesh::createMesh(verts, idxs, { {VERTEX_POS_COMPONENTS, GL_FLOAT}, {VERTEX_NORMAL_COMPONENTS, GL_FLOAT} });
}
