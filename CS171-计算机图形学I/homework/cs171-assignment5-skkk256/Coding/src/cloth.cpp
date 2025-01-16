#include "cloth.hpp"

RectCloth::
RectCloth(
    unsigned int nw,
    unsigned int nh,
    float dx,
    glm::mat4 transform
) : nw(nw), nh(nh), dx(dx), transform(transform), width((float)(nw - 1) * dx), height((float)(nh - 1) * dx)
{
    for (unsigned int ih = 0; ih < nh; ++ih) {
        for (unsigned int iw = 0; iw < nw; ++iw) {
            positions.push_back(getInitialPosition(iw, ih));
        }
    }
}

glm::vec3 RectCloth::
getInitialPosition(unsigned int iw, unsigned int ih) {
    return transform * glm::vec4((float)iw * dx - width / 2.0f, (float)ih * dx - height / 2.0f, 0.0f, 1.0f);
}
