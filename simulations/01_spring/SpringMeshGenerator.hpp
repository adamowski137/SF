#pragma once
#include <memory>
#include <vector>
#include "core/Mesh.hpp"

// Default spring configuration parameters
#define DEFAULT_SPRING_COIL_RADIUS        0.35f
#define DEFAULT_SPRING_WIRE_RADIUS        0.04f
#define DEFAULT_SPRING_NUM_COILS          10
#define DEFAULT_SPRING_SEGMENTS_PER_COIL  24
#define DEFAULT_SPRING_CIRCLE_SEGMENTS    10
#define DEFAULT_SPRING_REST_LENGTH        4.0f
#define DEFAULT_SPRING_MOUNT_Y            2.5f

// Geometry calculation defines
#define CUBE_HALF_EXTENT_FACTOR           0.5f

class SpringMeshGenerator {
public:
    struct SpringParams {
        float coilRadius = DEFAULT_SPRING_COIL_RADIUS;
        float wireRadius = DEFAULT_SPRING_WIRE_RADIUS;
        int numCoils = DEFAULT_SPRING_NUM_COILS;
        int segmentsPerCoil = DEFAULT_SPRING_SEGMENTS_PER_COIL;
        int circleSegments = DEFAULT_SPRING_CIRCLE_SEGMENTS;
        float restLength = DEFAULT_SPRING_REST_LENGTH;
        float mountY = DEFAULT_SPRING_MOUNT_Y;
    };

    SpringParams params;

    std::shared_ptr<Mesh> springMesh;
    std::shared_ptr<Mesh> mountMesh;
    std::shared_ptr<Mesh> rulerMesh;

    SpringMeshGenerator() = default;

    void init();
    void updateSpring(float w_val, float x_val);

    float getTopY(float w_val) const {
        return params.mountY + w_val;
    }

    float getBottomY(float x_val) const {
        return params.mountY - params.restLength + x_val;
    }

    float getCubeCenterY(float x_val, float cubeSize) const {
        return getBottomY(x_val) - CUBE_HALF_EXTENT_FACTOR * cubeSize;
    }

private:
    std::vector<float> scratchVerts;
    std::vector<unsigned int> scratchIndices;

    void generateSpringVertices(float yTop, float yBottom, std::vector<float>& vertices, std::vector<unsigned int>& indices);
    void buildRulerMesh();
};
