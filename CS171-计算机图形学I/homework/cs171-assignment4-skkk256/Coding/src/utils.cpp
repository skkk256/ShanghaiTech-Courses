#include <random>
#include <algorithm>
#include <cmath>
#include "utils.hpp"
#include "constant.hpp"


/**
 * StrUtils namespace
 */

void StrUtils::ltrim(std::string& s)
{
   s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
}

void StrUtils::rtrim(std::string& s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
}

void StrUtils::trim(std::string& s)
{
    ltrim(s);
    rtrim(s);
}


/**
 * MathUtils namespace
 */

float MathUtils::clamp(float x, float lo, float hi)
{
    return x < lo ? lo : x > hi ? hi : x;
}

unsigned char MathUtils::gamma_correction(float x)
{
	return (unsigned char)(pow(MathUtils::clamp(x, 0.0, 1.0), 1 / 2.2) * 255);
}
