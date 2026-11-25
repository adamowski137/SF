#pragma once
#include "JellyPhysics.hpp"
#include <glm/glm.hpp>
#include <array>
#include <vector>

class JellyMeshGenerator {
public:
    // Interleaved vertex data: position (3) + normal (3)
    struct MeshData {
        std::vector<float> vertices;
        std::vector<unsigned> indices;
    };

    // Boundary of the unit cube C = [0,1]^3 as 6 grids (div x div), outward normals, CCW winding
    static MeshData GetUnitCubeSurface(int div);

    // Torus (complex triangle mesh) inscribed in C = [0,1]^3
    static MeshData GetTorusInUnitCube(float r, float R, int div_r, int div_R);

    // Maps rest vertices from C through the Bezier volume F(u,v,w) = sum P_ijk B_i(u) B_j(v) B_k(w)
    static std::vector<float> Deform(const std::array<glm::vec3, JellyPhysics::COUNT>& P, const std::vector<float>& rest);

    // Line indices of the 12 edges of a cube whose corners n = 0..7 are stored at offset + n (bits = x, y, z)
    static std::vector<unsigned> GetCubeEdgeIndices(unsigned offset = 0);

    // Positions -> flat float array (for line meshes)
    static std::vector<float> Flatten(const glm::vec3* points, size_t count);
};
