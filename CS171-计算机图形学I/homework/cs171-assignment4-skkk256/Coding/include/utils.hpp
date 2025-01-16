#pragma once
#include <iostream>
#include <string>
#include <vector>
#include "Eigen/Dense"

#define UNREACHABLE std::cout << "Error: Unreachable code executed. Exit(-1)..." << std::endl; exit(-1);


/**
 * String processing utilities
 */

namespace StrUtils
{
    /* Trim from left */
    void ltrim(std::string& s);
    /* Trim from right */
    void rtrim(std::string& s);
    /* Trim from both left and right */
    void trim(std::string& s);
};


/**
 * Mathematical utilities
 */

namespace MathUtils
{
    /* Clamps the input x to the closed range [lo, hi] */
    float clamp(float x, float lo, float hi);
    /* Performs Gamma correction on x and outputs an integer between 0-255. */
    unsigned char gamma_correction(float x);
};
