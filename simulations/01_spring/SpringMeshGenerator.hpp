#pragma once
#include <memory>
#include <vector>
#include "core/Mesh.hpp"

class SpringMeshGenerator {
public:
    struct SpringParams {
        float coilRadius = 0.35f;
        float wireRadius = 0.04f;
        int numCoils = 10;
        int segmentsPerCoil = 24;
        int circleSegments = 10;
        float restLength = 4.0f;
        float mountY = 2.5f;
    };

    SpringParams params;

    std::shared_ptr<Mesh> springMesh;
    std::shared_ptr<Mesh> mountMesh;
    std::shared_ptr<Mesh> rulerMesh;

    SpringMeshGenerator();

    void init();
    void updateSpring(float w_val, float x_val);

    float getTopY(float w_val) const {
        return params.mountY + w_val;
    }

    float getBottomY(float x_val) const {
        return params.mountY - params.restLength + x_val;
    }

    float getCubeCenterY(float x_val, float cubeSize) const {
        return getBottomY(x_val) - 0.5f * cubeSize;
    }

private:
    void generateSpringVertices(float yTop, float yBottom, std::vector<float>& vertices, std::vector<unsigned int>& indices);
    void buildRulerMesh();
};
