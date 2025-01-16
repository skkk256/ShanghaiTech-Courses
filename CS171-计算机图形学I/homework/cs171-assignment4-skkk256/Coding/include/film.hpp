#pragma once
#include <vector>
#include "Eigen/Dense"
#include "utils.hpp"


/**
 * A data structure for storing imaging pixels
 */
struct Film
{
    Eigen::Vector2i resolution;
    std::vector<Eigen::Vector3f> pixel_array;

    Film(const Eigen::Vector2i& res);
	float getAspectRatio() const;
    void writeToFile(const std::string& output_path) const;
};
