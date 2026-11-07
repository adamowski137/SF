#pragma once
#include <memory>
#include <vector>
#include <glm/glm.hpp>

class Mesh;

class CubeMeshGenerator {
public:
    // Solid cube with corner at (0, 0, 0) extending to (1, 1, 1), with face normals
    static std::shared_ptr<Mesh> GetCornerCubeMesh();

    // Horizontal quad mesh (semi-transparent plane perpendicular to vertical/gravity)
    static std::shared_ptr<Mesh> GetPlaneMesh(float size = 8.0f);

    // Coordinate grid mesh for the ground plane
    static std::shared_ptr<Mesh> GetGridMesh(float size = 10.0f, float step = 0.5f);

    // Cone mesh (base at Y=0, apex at Y=h, radius r)
    static std::shared_ptr<Mesh> GetConeMesh(float r = 0.08f, float h = 0.2f, int div_circ = 24);

    // Simple 2-point unit line (0,0,0) to (0,1,0)
    static std::shared_ptr<Mesh> GetUnitLineMesh();

    // Trajectory dynamic mesh updater
    static std::shared_ptr<Mesh> CreateTrajectoryMesh();
    static void UpdateTrajectoryMesh(const std::shared_ptr<Mesh>& mesh, const std::vector<glm::vec3>& points);

    // Helper to compute transformation matrix that maps unit cylinder (centered at Y=0, height=1)
    // to connect point A to point B with specified radius
    static glm::mat4 ComputeCylinderTransform(const glm::vec3& pA, const glm::vec3& pB, float radius);

    // Helper to compute transformation matrix that places a unit cone/arrowhead at tip pointing in direction
    static glm::mat4 ComputeConeTransform(const glm::vec3& tipPos, const glm::vec3& dir, float length, float radius);
};
