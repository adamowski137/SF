#include "JellyMeshGenerator.hpp"
#include "core/MeshGenerator.hpp"

namespace {
    // Cubic Bernstein basis B_i(t) and its derivative B_i'(t)
    void bernstein(float t, float B[4], float dB[4]) {
        float s = 1.0f - t;
        B[0] = s * s * s;
        B[1] = 3.0f * t * s * s;
        B[2] = 3.0f * t * t * s;
        B[3] = t * t * t;
        dB[0] = -3.0f * s * s;
        dB[1] = 3.0f * s * s - 6.0f * t * s;
        dB[2] = 6.0f * t * s - 3.0f * t * t;
        dB[3] = 3.0f * t * t;
    }
}

JellyMeshGenerator::MeshData JellyMeshGenerator::GetUnitCubeSurface(int div) {
    MeshData data;
    for (int ax = 0; ax < 3; ++ax) {
        for (int side = 0; side < 2; ++side) {
            // Tangent axes ordered so that cross(t1, t2) points outward (cyclic order for +, swapped for -)
            int t1 = (ax + (side ? 1 : 2)) % 3;
            int t2 = (ax + (side ? 2 : 1)) % 3;
            glm::vec3 n(0.0f);
            n[ax] = side ? 1.0f : -1.0f;

            unsigned base = static_cast<unsigned>(data.vertices.size() / 6);
            for (int y = 0; y <= div; ++y) {
                for (int x = 0; x <= div; ++x) {
                    glm::vec3 q(0.0f);
                    q[ax] = static_cast<float>(side);
                    q[t1] = static_cast<float>(x) / div;
                    q[t2] = static_cast<float>(y) / div;
                    data.vertices.insert(data.vertices.end(), { q.x, q.y, q.z, n.x, n.y, n.z });
                }
            }
            for (int y = 0; y < div; ++y) {
                for (int x = 0; x < div; ++x) {
                    unsigned i0 = base + y * (div + 1) + x;
                    unsigned i1 = i0 + 1;
                    unsigned i2 = i0 + div + 2;
                    unsigned i3 = i0 + div + 1;
                    data.indices.insert(data.indices.end(), { i0, i1, i2, i0, i2, i3 });
                }
            }
        }
    }
    return data;
}

JellyMeshGenerator::MeshData JellyMeshGenerator::GetTorusInUnitCube(float r, float R, int div_r, int div_R) {
    auto [vertices, indices] = MeshGenerator::GetTorusVerticies(r, R, div_r, div_R);

    // Uniform scale + shift into [0,1]^3 with a small margin (normals unchanged)
    float s = 0.45f / (R + r);
    for (size_t o = 0; o < vertices.size(); o += 6)
        for (int c = 0; c < 3; ++c)
            vertices[o + c] = 0.5f + s * vertices[o + c];

    return { vertices, indices };
}

std::vector<float> JellyMeshGenerator::Deform(const std::array<glm::vec3, JellyPhysics::COUNT>& P, const std::vector<float>& rest) {
    std::vector<float> out(rest.size());
    float Bu[4], Bv[4], Bw[4], dBu[4], dBv[4], dBw[4];

    for (size_t o = 0; o < rest.size(); o += 6) {
        bernstein(rest[o + 0], Bu, dBu);
        bernstein(rest[o + 1], Bv, dBv);
        bernstein(rest[o + 2], Bw, dBw);

        // F and its partial derivatives (columns of the Jacobian J = [Fu Fv Fw])
        glm::vec3 F(0.0f), Fu(0.0f), Fv(0.0f), Fw(0.0f);
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                for (int k = 0; k < 4; ++k) {
                    const glm::vec3& p = P[JellyPhysics::idx(i, j, k)];
                    F  += p * (Bu[i]  * Bv[j]  * Bw[k]);
                    Fu += p * (dBu[i] * Bv[j]  * Bw[k]);
                    Fv += p * (Bu[i]  * dBv[j] * Bw[k]);
                    Fw += p * (Bu[i]  * Bv[j]  * dBw[k]);
                }

        // Normals from the Bernstein definition: n' ~ J^-T n = cofactor(J) n / det(J)
        // (left unnormalized - the phong shader normalizes)
        glm::vec3 n(rest[o + 3], rest[o + 4], rest[o + 5]);
        glm::vec3 N = glm::cross(Fv, Fw) * n.x + glm::cross(Fw, Fu) * n.y + glm::cross(Fu, Fv) * n.z;

        out[o + 0] = F.x; out[o + 1] = F.y; out[o + 2] = F.z;
        out[o + 3] = N.x; out[o + 4] = N.y; out[o + 5] = N.z;
    }
    return out;
}

std::vector<unsigned> JellyMeshGenerator::GetCubeEdgeIndices(unsigned offset) {
    std::vector<unsigned> indices;
    for (unsigned n = 0; n < 8; ++n)
        for (unsigned bit : { 1u, 2u, 4u })
            if (!(n & bit)) // edge to the neighbouring corner differing in one coordinate
                indices.insert(indices.end(), { offset + n, offset + (n | bit) });
    return indices;
}

std::vector<float> JellyMeshGenerator::Flatten(const glm::vec3* points, size_t count) {
    std::vector<float> out;
    out.reserve(count * 3);
    for (size_t i = 0; i < count; ++i)
        out.insert(out.end(), { points[i].x, points[i].y, points[i].z });
    return out;
}
