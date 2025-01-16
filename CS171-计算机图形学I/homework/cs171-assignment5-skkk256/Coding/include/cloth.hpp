#pragma once

#include <vector>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

struct RectCloth {
private:
    std::vector<glm::vec3> positions;

public:
    const float dx;
    const unsigned int nw;
    const unsigned int nh;
    const glm::mat4 transform;

    const float width;
    const float height;

public:
    RectCloth(unsigned int nw, unsigned int nh, float dx, glm::mat4 transform = glm::mat4(1.0));
    ~RectCloth() = default;

    unsigned int idxFromCoord(unsigned int iw, unsigned int ih) { return ih * nw + iw; };

    glm::vec3 getInitialPosition(unsigned int iw, unsigned int ih);

    glm::vec3& getPosition(unsigned int idx) { return positions[idx]; };
    glm::vec3& getPosition(unsigned int iw, unsigned int ih) { return positions[idxFromCoord(iw, ih)]; };

    void setPosition(unsigned int idx, const glm::vec3& value) { positions[idx] = value; };
    void setPosition(unsigned int iw, unsigned int ih, const glm::vec3& value) { positions[idxFromCoord(iw, ih)] = value; };
};