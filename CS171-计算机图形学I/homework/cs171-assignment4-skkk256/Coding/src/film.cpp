#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "film.hpp"
#include "utils.hpp"


/**
 * Film class
 */

Film::Film(const Eigen::Vector2i& res)
    : resolution(res)
{
    pixel_array.resize(resolution.x() * resolution.y(), Eigen::Vector3f(1, 1, 1));
}

float Film::getAspectRatio() const
{
    return (float)(resolution.x()) / (float)(resolution.y());
}

void Film::writeToFile(const std::string& output_path) const
{
    std::vector<unsigned char> output_data;
    output_data.reserve(int(resolution.x() * resolution.y() * 3));
    for (Eigen::Vector3f v : pixel_array)
    {
        for (int i = 0; i < 3; i++)
        {
            output_data.push_back(MathUtils::gamma_correction(v[i]));
        }
    }
    stbi_flip_vertically_on_write(true);
    stbi_write_png(output_path.c_str(), resolution.x(), resolution.y(), 3, output_data.data(), 0);
}

