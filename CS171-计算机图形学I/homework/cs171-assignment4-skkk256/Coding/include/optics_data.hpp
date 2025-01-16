#pragma once
#include "Eigen/Dense"


struct OpticsData
{
    Eigen::Vector3f transparency = Eigen::Vector3f(1.0f, 1.0f, 1.0f);
    Eigen::Vector3f color = Eigen::Vector3f(0, 0, 0);
    Eigen::Vector3f getColor() const
    {
        return color;
    }
    Eigen::Vector3f getOpacity()
    {
        return Eigen::Vector3f::Ones() - this->transparency;
    }
};