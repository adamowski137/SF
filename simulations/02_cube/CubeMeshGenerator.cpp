#include "CubeMeshGenerator.hpp"
#include "core/Mesh.hpp"
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace {
    glm::quat rotationBetweenVectors(const glm::vec3& start, const glm::vec3& dest) {
        glm::vec3 u = glm::normalize(start);
        glm::vec3 v = glm::normalize(dest);
        float cosTheta = glm::clamp(glm::dot(u, v), -1.0f, 1.0f);

        if (cosTheta > 0.999999f) {
            return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        }
        if (cosTheta < -0.999999f) {
            glm::vec3 axis = glm::cross(glm::vec3(1.0f, 0.0f, 0.0f), u);
            if (glm::length(axis) < 0.01f) {
                axis = glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), u);
            }
            return glm::angleAxis(static_cast<float>(M_PI), glm::normalize(axis));
        }

        glm::vec3 axis = glm::cross(u, v);
        float angle = std::acos(cosTheta);
        return glm::angleAxis(angle, glm::normalize(axis));
    }
}

std::shared_ptr<Mesh> CubeMeshGenerator::GetCornerCubeMesh() {
    // 6 faces * 4 vertices = 24 vertices
    // Position (3) + Normal (3) = 6 floats per vertex
    std::vector<float> vertices = {
        // Front face (Z = 1, normal 0,0,1)
        0.0f, 0.0f, 1.0f,   0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f,   0.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f,   0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 1.0f,   0.0f, 0.0f, 1.0f,

        // Back face (Z = 0, normal 0,0,-1)
        1.0f, 0.0f, 0.0f,   0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, 0.0f,   0.0f, 0.0f, -1.0f,
        0.0f, 1.0f, 0.0f,   0.0f, 0.0f, -1.0f,
        1.0f, 1.0f, 0.0f,   0.0f, 0.0f, -1.0f,

        // Top face (Y = 1, normal 0,1,0)
        0.0f, 1.0f, 1.0f,   0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f,   0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,   0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,   0.0f, 1.0f, 0.0f,

        // Bottom face (Y = 0, normal 0,-1,0)
        0.0f, 0.0f, 0.0f,   0.0f, -1.0f, 0.0f,
        1.0f, 0.0f, 0.0f,   0.0f, -1.0f, 0.0f,
        1.0f, 0.0f, 1.0f,   0.0f, -1.0f, 0.0f,
        0.0f, 0.0f, 1.0f,   0.0f, -1.0f, 0.0f,

        // Right face (X = 1, normal 1,0,0)
        1.0f, 0.0f, 1.0f,   1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f,   1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 1.0f,   1.0f, 0.0f, 0.0f,

        // Left face (X = 0, normal -1,0,0)
        0.0f, 0.0f, 0.0f,   -1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f,   -1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 1.0f,   -1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,   -1.0f, 0.0f, 0.0f,
    };

    std::vector<unsigned> indices;
    for (unsigned face = 0; face < 6; ++face) {
        unsigned b = face * 4;
        indices.push_back(b + 0);
        indices.push_back(b + 1);
        indices.push_back(b + 2);
        indices.push_back(b + 2);
        indices.push_back(b + 3);
        indices.push_back(b + 0);
    }

    return Mesh::createMesh(vertices, indices, { {3, GL_FLOAT}, {3, GL_FLOAT} });
}

std::shared_ptr<Mesh> CubeMeshGenerator::GetPlaneMesh(float size) {
    float h = size * 0.5f;
    std::vector<float> vertices = {
        -h, 0.0f, -h,   0.0f, 1.0f, 0.0f,
         h, 0.0f, -h,   0.0f, 1.0f, 0.0f,
         h, 0.0f,  h,   0.0f, 1.0f, 0.0f,
        -h, 0.0f,  h,   0.0f, 1.0f, 0.0f,
    };

    // Double-sided rendering indices
    std::vector<unsigned> indices = {
        0, 1, 2,  0, 2, 3, // top side
        0, 2, 1,  0, 3, 2  // bottom side
    };

    return Mesh::createMesh(vertices, indices, { {3, GL_FLOAT}, {3, GL_FLOAT} });
}

std::shared_ptr<Mesh> CubeMeshGenerator::GetGridMesh(float size, float step) {
    std::vector<float> vertices;
    std::vector<unsigned> indices;

    float half = size * 0.5f;
    unsigned idx = 0;

    for (float x = -half; x <= half + 1e-4f; x += step) {
        vertices.push_back(x); vertices.push_back(0.0f); vertices.push_back(-half);
        vertices.push_back(x); vertices.push_back(0.0f); vertices.push_back(half);
        indices.push_back(idx++);
        indices.push_back(idx++);
    }

    for (float z = -half; z <= half + 1e-4f; z += step) {
        vertices.push_back(-half); vertices.push_back(0.0f); vertices.push_back(z);
        vertices.push_back(half);  vertices.push_back(0.0f); vertices.push_back(z);
        indices.push_back(idx++);
        indices.push_back(idx++);
    }

    return Mesh::createMesh(vertices, indices, { {3, GL_FLOAT} });
}

std::shared_ptr<Mesh> CubeMeshGenerator::GetConeMesh(float r, float h, int div_circ) {
    std::vector<float> vertices;
    std::vector<unsigned> indices;

    // Side vertices: apex + ring
    int apex_idx = 0;
    vertices.push_back(0.0f); vertices.push_back(h); vertices.push_back(0.0f);
    vertices.push_back(0.0f); vertices.push_back(1.0f); vertices.push_back(0.0f);

    float slope = r / h;
    for (int i = 0; i <= div_circ; ++i) {
        float theta = i * 2.0f * static_cast<float>(M_PI) / div_circ;
        float x = r * std::cos(theta);
        float z = r * std::sin(theta);
        glm::vec3 norm = glm::normalize(glm::vec3(std::cos(theta), slope, std::sin(theta)));
        vertices.push_back(x); vertices.push_back(0.0f); vertices.push_back(z);
        vertices.push_back(norm.x); vertices.push_back(norm.y); vertices.push_back(norm.z);
    }

    for (int i = 0; i < div_circ; ++i) {
        indices.push_back(apex_idx);
        indices.push_back(i + 1);
        indices.push_back(i + 2);
    }

    // Base cap
    int base_center_idx = static_cast<int>(vertices.size() / 6);
    vertices.push_back(0.0f); vertices.push_back(0.0f); vertices.push_back(0.0f);
    vertices.push_back(0.0f); vertices.push_back(-1.0f); vertices.push_back(0.0f);

    int base_ring_start = static_cast<int>(vertices.size() / 6);
    for (int i = 0; i <= div_circ; ++i) {
        float theta = i * 2.0f * static_cast<float>(M_PI) / div_circ;
        float x = r * std::cos(theta);
        float z = r * std::sin(theta);
        vertices.push_back(x); vertices.push_back(0.0f); vertices.push_back(z);
        vertices.push_back(0.0f); vertices.push_back(-1.0f); vertices.push_back(0.0f);
    }

    for (int i = 0; i < div_circ; ++i) {
        indices.push_back(base_center_idx);
        indices.push_back(base_ring_start + i + 1);
        indices.push_back(base_ring_start + i);
    }

    return Mesh::createMesh(vertices, indices, { {3, GL_FLOAT}, {3, GL_FLOAT} });
}

std::shared_ptr<Mesh> CubeMeshGenerator::GetUnitLineMesh() {
    std::vector<float> vertices = {
        0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f
    };
    std::vector<unsigned> indices = { 0, 1 };
    return Mesh::createMesh(vertices, indices, { {3, GL_FLOAT} });
}

std::shared_ptr<Mesh> CubeMeshGenerator::CreateTrajectoryMesh() {
    std::vector<float> vertices = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
    std::vector<unsigned> indices = { 0, 1 };
    return Mesh::createMesh(vertices, indices, { {3, GL_FLOAT} });
}

void CubeMeshGenerator::UpdateTrajectoryMesh(const std::shared_ptr<Mesh>& mesh, const std::vector<glm::vec3>& points) {
    if (!mesh || points.size() < 2) return;

    std::vector<float> vertices;
    vertices.reserve(points.size() * 3);
    for (const auto& p : points) {
        vertices.push_back(p.x);
        vertices.push_back(p.y);
        vertices.push_back(p.z);
    }

    std::vector<unsigned> indices;
    indices.reserve((points.size() - 1) * 2);
    for (unsigned i = 0; i + 1 < points.size(); ++i) {
        indices.push_back(i);
        indices.push_back(i + 1);
    }

    mesh->updateMesh(vertices, indices);
}

glm::mat4 CubeMeshGenerator::ComputeCylinderTransform(const glm::vec3& pA, const glm::vec3& pB, float radius) {
    glm::vec3 dir = pB - pA;
    float len = glm::length(dir);
    if (len < 1e-6f) {
        return glm::translate(glm::mat4(1.0f), pA);
    }

    glm::vec3 mid = 0.5f * (pA + pB);
    glm::quat rot = rotationBetweenVectors(glm::vec3(0.0f, 1.0f, 0.0f), dir / len);

    glm::mat4 model = glm::translate(glm::mat4(1.0f), mid) *
                      glm::mat4_cast(rot) *
                      glm::scale(glm::mat4(1.0f), glm::vec3(radius, len, radius));
    return model;
}

glm::mat4 CubeMeshGenerator::ComputeConeTransform(const glm::vec3& tipPos, const glm::vec3& dir, float length, float radius) {
    float dirLen = glm::length(dir);
    if (dirLen < 1e-6f) {
        return glm::translate(glm::mat4(1.0f), tipPos);
    }

    glm::vec3 normDir = dir / dirLen;
    glm::quat rot = rotationBetweenVectors(glm::vec3(0.0f, 1.0f, 0.0f), normDir);

    // Tip is at tipPos, base is at tipPos - normDir * length
    glm::vec3 base = tipPos - normDir * length;
    glm::vec3 mid = 0.5f * (base + tipPos);

    glm::mat4 model = glm::translate(glm::mat4(1.0f), mid) *
                      glm::mat4_cast(rot) *
                      glm::scale(glm::mat4(1.0f), glm::vec3(radius, length, radius));
    return model;
}
