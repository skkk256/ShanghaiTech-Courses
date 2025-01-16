#ifndef _BSPLINE_H_
#define _BSPLINE_H_
#include "defines.h"
#include <object.h>

#include <vector>

class BSplineCurve {
public:
    int degree = 2;
    std::vector<float> knot;
    std::vector<vec3> control_points_;

    BSplineCurve(std::vector<vec3>& control_points);
    vec3 evaluate(float t);
    vec3 evaluate(std::vector<vec3>& control_points, float t);
};

class BSplineSurface {
public:
    std::vector<std::vector<vec3>> control_points_m_;
    std::vector<std::vector<vec3>> control_points_n_;

    BSplineSurface(std::vector<std::vector<vec3>>& control_points);
    BSplineSurface(int m, int n);

    Vertex evaluate(std::vector<std::vector<vec3>>& control_points, float u, float v);
    Vertex evaluate(float u, float v);
    Object generateObject(float resolution);
};

#endif