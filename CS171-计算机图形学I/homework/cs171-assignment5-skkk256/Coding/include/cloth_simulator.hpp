#pragma once

#include "cloth.hpp"
#include "mesh.h"

enum Direction{
    UP, DOWN, FORWARD, BACKWARD, LEFT, RIGHT
};

class RectClothSimulator {
private:
    struct MassParticle {
        glm::vec3 position;
        std::vector<unsigned int> connectedSpringStartIndices;
        std::vector<unsigned int> connectedSpringEndIndices;

        // TODO: define other particle properties here
        glm::vec3 velocity;
        float mass;
    };

    struct Spring
    {
        unsigned int fromMassIndex;
        unsigned int toMassIndex;

        // TODO: define other spring properties here
        float stiffness;
        float currentLength;
        float initLength;

    };

    RectCloth* cloth;
    Mesh* mesh;
    std::vector<MassParticle> particles;
    std::vector<Spring> springs;

    // Simulation parameters
    glm::vec3 gravity;
    glm::vec3 wind;
    float airResistanceCoefficient; // Per-particle

public:
    RectClothSimulator(
            RectCloth* cloth,
            Mesh* mesh,
            float totalMass,
            float stiffnessReference,
            float airResistanceCoefficient,
            const glm::vec3& gravity,
            const glm::vec3& wind);
    ~RectClothSimulator() = default;

    void step(float timeStep);
    void movePoint(Direction direction);

private:
    void createMassParticles(float totalMass);
    void createSprings(float stiffnessReference);
    void updateCloth();
};