#include "cloth_simulator.hpp"
#include <iostream>

RectClothSimulator::
RectClothSimulator(
        RectCloth *cloth,
        Mesh* mesh,
        float totalMass,
        float stiffnessReference,
        float airResistanceCoefficient,
        const glm::vec3& gravity,
        const glm::vec3& wind) : cloth(cloth), mesh(mesh), airResistanceCoefficient(airResistanceCoefficient), gravity(gravity), wind(wind) {
    // Initialize particles, then springs according to the given cloth
    createMassParticles(totalMass);
    createSprings(stiffnessReference);
}

void RectClothSimulator::
createMassParticles(float totalMass) {
    // Create mass particles based on given cloth.
    particles.resize(cloth->nw * cloth->nh);
    float particleMass = totalMass / (cloth->nw * cloth->nh);
    for (int ih = 0; ih < cloth->nh; ih++) {
        for (int iw = 0; iw < cloth->nw; iw++) {
            MassParticle particle;
            particle.position = cloth->getPosition(iw, ih);

            // TODO: Initialize other mass properties.
            //  Use 'cloth->...' to access cloth properties.
            particle.velocity = glm::vec3(0.0f, 0.0f, 0.0f); // initial velocity is zero
            particle.mass = particleMass; // set mass of particle
            particles[cloth->idxFromCoord(iw, ih)] = particle;
        }
    }
}

void RectClothSimulator::
createSprings(float stiffnessReference) {
    // First clear all springs
    springs.clear();

    // TODO: Create springs connecting mass particles.
    //  You may find 'cloth->idxFromCoord(...)' useful.
    //  You can store springs into the member variable 'springs' which is a std::vector.
    //  You may want to modify mass particles too.
    for (int ih = 0; ih < cloth->nh; ih++) {
        for (int iw = 0; iw < cloth->nw; iw++) {
            unsigned int idx = cloth->idxFromCoord(iw, ih);

            // Define the eight neighboring particles
            std::vector<std::vector<int>> neighbors = {
                    {iw - 1, ih - 1}, {iw, ih - 1}, {iw + 1, ih - 1},
                    {iw - 1, ih},                   {iw + 1, ih},
                    {iw - 1, ih + 1}, {iw, ih + 1}, {iw + 1, ih + 1}
            };

            for (int i = 0; i < 8; i++) {
                int nw = neighbors[i][0];
                int nh = neighbors[i][1];

                // Check if the neighbor is within the cloth
                if (nw >= 0 && nw < cloth->nw && nh >= 0 && nh < cloth->nh) {
                    Spring spring;
                    spring.fromMassIndex = idx;
                    spring.toMassIndex = cloth->idxFromCoord(nw, nh);
                    spring.stiffness = stiffnessReference; // Set the spring's stiffness

                    // Initialize the current length of the spring
                    spring.currentLength = glm::distance(particles[spring.fromMassIndex].position, particles[spring.toMassIndex].position);
                    spring.initLength = spring.currentLength;

                    springs.push_back(spring);

                    // Update the connected springs of the two particles
                    particles[spring.fromMassIndex].connectedSpringEndIndices.push_back(springs.size() - 1);
                    particles[spring.toMassIndex].connectedSpringStartIndices.push_back(springs.size() - 1);
                }
            }
        }
    }
}

void RectClothSimulator::
step(float timeStep) {
    // TODO: Simulate one step based on given time step.
    //  Step 1: Update particle positions
    //  Step 2: Update springs
    //  Step 3: Apply constraints
    //  Hint: See cloth_simulator.hpp to check for member variables you need.
    //  Hint: You may use 'cloth->getInitialPosition(...)' for constraints.
    // Step 1: Update particle positions
    for (auto& particle : particles) {
        glm::vec3 netForce = particle.mass * gravity; // gravitational force
        netForce -= airResistanceCoefficient * glm::length(particle.velocity) * particle.velocity; // air resistance
        netForce += wind; // wind

        // spring forces
        for (auto springIndex : particle.connectedSpringEndIndices) {
            const Spring& spring = springs[springIndex];
            glm::vec3 springVector = particles[spring.toMassIndex].position - particle.position;
//            std::cout << particles[spring.toMassIndex].position.x << " " << particles[spring.toMassIndex].position.y << " " << particles[spring.toMassIndex].position.z << std::endl

            float currentLength = spring.currentLength;
            float initLength = spring.initLength;
//            std::cout << currentLength << " " << currentLength2 << std::endl;

            netForce += spring.stiffness * (currentLength - initLength) * glm::normalize(springVector);
//            std::cout << netForce.x << " " << netForce.y << " " << netForce.z << std::endl;
        }

        // symplectic Euler's method
        glm::vec3 acceleration = netForce / particle.mass;
        particle.velocity += timeStep * acceleration;

        glm::vec3 sphereCenter = mesh->getCenter();
        glm::vec3 collisionVector = particle.position + timeStep * particle.velocity - sphereCenter;
//        std::cout << glm::length(collisionVector) << std::endl;
        if (glm::length(collisionVector) > mesh->getRadius() + 0.01) {
            particle.position += timeStep * particle.velocity;
        }
        else
        {
            particle.velocity = glm::vec3(0.0f, 0.0f, 0.0f);
        }
    }

    for (auto& spring : springs) {
        spring.currentLength = glm::length(particles[spring.toMassIndex].position - particles[spring.fromMassIndex].position);
    }

    particles[cloth->idxFromCoord(0, 0)].position = cloth->getInitialPosition(0, 0);
//    particles[cloth->idxFromCoord(0, 0)].position =  glm::vec3(0.0f, -1.35f, 0.0f);
    particles[cloth->idxFromCoord(cloth->nw - 1, 0)].position = cloth->getInitialPosition(cloth->nw - 1, 0);

    // Finally update cloth data
    updateCloth();
}

void RectClothSimulator::
updateCloth() {
    for (unsigned int i = 0u; i < cloth->nw * cloth->nh; i++)
    {
        cloth->setPosition(i, particles[i].position);
    }
}


void RectClothSimulator::movePoint(Direction direction) {
    switch (direction) {
        case UP:
            particles[cloth->idxFromCoord(cloth->nw - 1, cloth->nh-1)].position += glm::vec3{0.0f, 0.01f, 0.0f};
            break;
        case DOWN:
            particles[cloth->idxFromCoord(cloth->nw - 1, cloth->nh-1)].position += glm::vec3{0.0f, -0.01f, 0.0f};
            break;
        case FORWARD:
            particles[cloth->idxFromCoord(cloth->nw - 1, cloth->nh-1)].position += glm::vec3{0.0f, 0.0f, 0.01f};
            break;
        case BACKWARD:
            particles[cloth->idxFromCoord(cloth->nw - 1, cloth->nh-1)].position += glm::vec3{0.0f, 0.01f, -0.01f};
            break;
        case LEFT:
            particles[cloth->idxFromCoord(cloth->nw - 1, cloth->nh-1)].position += glm::vec3{-0.01f, 0.0f, 0.0f};
            break;
        case RIGHT:
            particles[cloth->idxFromCoord(cloth->nw - 1, cloth->nh-1)].position += glm::vec3{0.01f, 0.01f, 0.0f};
            break;
    }
}