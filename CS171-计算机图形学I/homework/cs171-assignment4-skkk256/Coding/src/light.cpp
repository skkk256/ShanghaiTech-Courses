#include "light.hpp"


/**
 * Light class
 */

Light::Light(Eigen::Vector3f pos, Eigen::Vector3f rgb)
    : position(pos)
    , color(rgb)
{
}

Eigen::Vector3f Light::getPosition() const
{
    return position;
}

Eigen::Vector3f Light::getColor() const
{
    return color;
}
