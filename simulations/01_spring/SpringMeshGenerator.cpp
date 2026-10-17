#include "SpringMeshGenerator.hpp"
#include "core/MeshGenerator.hpp"
#include <cmath>

SpringMeshGenerator::SpringMeshGenerator() {
}

void SpringMeshGenerator::init() {
    mountMesh = MeshGenerator::GetCylinderMesh(0.45f, 0.12f, 24, 2);
    buildRulerMesh();

    std::vector<float> verts;
    std::vector<unsigned int> indices;
    generateSpringVertices(getTopY(0.0f), getBottomY(0.0f), verts, indices);
    springMesh = Mesh::createMesh(verts, indices, { {3, GL_FLOAT}, {3, GL_FLOAT} });
}

void SpringMeshGenerator::updateSpring(float w_val, float x_val) {
    if (!springMesh) return;

    float yTop = getTopY(w_val);
    float yBottom = getBottomY(x_val);

    std::vector<float> verts;
    std::vector<unsigned int> indices;
    generateSpringVertices(yTop, yBottom, verts, indices);

    springMesh->updateMesh(verts, indices);
}

void SpringMeshGenerator::generateSpringVertices(
    float yTop, float yBottom, std::vector<float>& vertices, std::vector<unsigned int>& indices)
{
    const int totalSlices = params.numCoils * params.segmentsPerCoil;
    const int M = params.circleSegments;
    const float twoPi = 6.28318530718f;

    vertices.clear();
    indices.clear();

    // Generate path points and coordinate frames
    struct PathNode {
        float x, y, z;
        float tx, ty, tz;
        float nx, ny, nz;
        float bx, by, bz;
    };

    std::vector<PathNode> path(totalSlices + 1);

    for (int i = 0; i <= totalSlices; i++) {
        float s = static_cast<float>(i) / static_cast<float>(totalSlices);
        float y = yTop + s * (yBottom - yTop);
        float angle = twoPi * params.numCoils * s;

        // Smooth taper at the very ends (0% to 3% and 97% to 100%)
        float taper = 1.0f;
        if (s < 0.04f) {
            taper = s / 0.04f;
        } else if (s > 0.96f) {
            taper = (1.0f - s) / 0.04f;
        }

        float R = params.coilRadius * taper;
        path[i].x = R * std::cos(angle);
        path[i].y = y;
        path[i].z = R * std::sin(angle);
    }

    // Compute tangents, normals, binormals
    for (int i = 0; i <= totalSlices; i++) {
        float tx, ty, tz;
        if (i == 0) {
            tx = path[1].x - path[0].x;
            ty = path[1].y - path[0].y;
            tz = path[1].z - path[0].z;
        } else if (i == totalSlices) {
            tx = path[totalSlices].x - path[totalSlices - 1].x;
            ty = path[totalSlices].y - path[totalSlices - 1].y;
            tz = path[totalSlices].z - path[totalSlices - 1].z;
        } else {
            tx = path[i + 1].x - path[i - 1].x;
            ty = path[i + 1].y - path[i - 1].y;
            tz = path[i + 1].z - path[i - 1].z;
        }

        float tLen = std::sqrt(tx * tx + ty * ty + tz * tz);
        if (tLen > 1e-6f) { tx /= tLen; ty /= tLen; tz /= tLen; }
        else { tx = 0.0f; ty = -1.0f; tz = 0.0f; }

        path[i].tx = tx;
        path[i].ty = ty;
        path[i].tz = tz;

        // Radial reference vector
        float s = static_cast<float>(i) / static_cast<float>(totalSlices);
        float angle = twoPi * params.numCoils * s;
        float rx = std::cos(angle);
        float ry = 0.0f;
        float rz = std::sin(angle);

        // B = T x R
        float bx = ty * rz - tz * ry;
        float by = tz * rx - tx * rz;
        float bz = tx * ry - ty * rx;
        float bLen = std::sqrt(bx * bx + by * by + bz * bz);
        if (bLen > 1e-6f) { bx /= bLen; by /= bLen; bz /= bLen; }
        else { bx = 1.0f; by = 0.0f; bz = 0.0f; }

        // N = B x T
        float nx = by * tz - bz * ty;
        float ny = bz * tx - bx * tz;
        float nz = bx * ty - by * tx;
        float nLen = std::sqrt(nx * nx + ny * ny + nz * nz);
        if (nLen > 1e-6f) { nx /= nLen; ny /= nLen; nz /= nLen; }
        else { nx = 0.0f; ny = 0.0f; nz = 1.0f; }

        path[i].nx = nx; path[i].ny = ny; path[i].nz = nz;
        path[i].bx = bx; path[i].by = by; path[i].bz = bz;
    }

    // Build vertices around each slice
    const float rWire = params.wireRadius;
    for (int i = 0; i <= totalSlices; i++) {
        const auto& node = path[i];
        for (int j = 0; j < M; j++) {
            float phi = twoPi * static_cast<float>(j) / static_cast<float>(M);
            float cPhi = std::cos(phi);
            float sPhi = std::sin(phi);

            float normX = cPhi * node.nx + sPhi * node.bx;
            float normY = cPhi * node.ny + sPhi * node.by;
            float normZ = cPhi * node.nz + sPhi * node.bz;

            float posX = node.x + rWire * normX;
            float posY = node.y + rWire * normY;
            float posZ = node.z + rWire * normZ;

            // Position (3) + Normal (3)
            vertices.push_back(posX);
            vertices.push_back(posY);
            vertices.push_back(posZ);
            vertices.push_back(normX);
            vertices.push_back(normY);
            vertices.push_back(normZ);
        }
    }

    // Build tube indices
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

    // Top cap
    unsigned int topCenterIdx = static_cast<unsigned int>(vertices.size() / 6);
    vertices.push_back(path[0].x);
    vertices.push_back(path[0].y);
    vertices.push_back(path[0].z);
    vertices.push_back(-path[0].tx);
    vertices.push_back(-path[0].ty);
    vertices.push_back(-path[0].tz);

    for (int j = 0; j < M; j++) {
        int next_j = (j + 1) % M;
        indices.push_back(topCenterIdx);
        indices.push_back(next_j);
        indices.push_back(j);
    }

    // Bottom cap
    unsigned int bottomCenterIdx = static_cast<unsigned int>(vertices.size() / 6);
    int lastRingStart = totalSlices * M;
    vertices.push_back(path[totalSlices].x);
    vertices.push_back(path[totalSlices].y);
    vertices.push_back(path[totalSlices].z);
    vertices.push_back(path[totalSlices].tx);
    vertices.push_back(path[totalSlices].ty);
    vertices.push_back(path[totalSlices].tz);

    for (int j = 0; j < M; j++) {
        int next_j = (j + 1) % M;
        indices.push_back(bottomCenterIdx);
        indices.push_back(lastRingStart + j);
        indices.push_back(lastRingStart + next_j);
    }
}

void SpringMeshGenerator::buildRulerMesh() {
    // Generate guide ruler: vertical rod with tick marks
    std::vector<float> verts;
    std::vector<unsigned int> idxs;

    // We can draw reference ticks as small line markers or thin quads
    // equilibrium at y = params.mountY - params.restLength
    float eqY = params.mountY - params.restLength;
    float rulerX = -0.8f;

    for (int tick = -4; tick <= 4; tick++) {
        float y = eqY + static_cast<float>(tick);
        float tickLen = (tick == 0) ? 0.25f : ((tick % 2 == 0) ? 0.18f : 0.10f);

        // Tick mark cylinder or quad
        float z0 = -tickLen * 0.5f;
        float z1 = tickLen * 0.5f;
        float dy = 0.015f;

        unsigned int base = static_cast<unsigned int>(verts.size() / 6);
        // Quad with normal facing front (+X)
        verts.push_back(rulerX); verts.push_back(y - dy); verts.push_back(z0); verts.push_back(1.0f); verts.push_back(0.0f); verts.push_back(0.0f);
        verts.push_back(rulerX); verts.push_back(y - dy); verts.push_back(z1); verts.push_back(1.0f); verts.push_back(0.0f); verts.push_back(0.0f);
        verts.push_back(rulerX); verts.push_back(y + dy); verts.push_back(z1); verts.push_back(1.0f); verts.push_back(0.0f); verts.push_back(0.0f);
        verts.push_back(rulerX); verts.push_back(y + dy); verts.push_back(z0); verts.push_back(1.0f); verts.push_back(0.0f); verts.push_back(0.0f);

        idxs.push_back(base); idxs.push_back(base + 1); idxs.push_back(base + 2);
        idxs.push_back(base); idxs.push_back(base + 2); idxs.push_back(base + 3);
    }

    rulerMesh = Mesh::createMesh(verts, idxs, { {3, GL_FLOAT}, {3, GL_FLOAT} });
}
