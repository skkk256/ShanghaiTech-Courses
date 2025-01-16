#ifndef _BEZIER_H_
#define _BEZIER_H_
#include "defines.h"
#include <object.h>

#include <vector>

class BezierCurve {
 public:
  std::vector<vec3> control_points_;

  BezierCurve(int m);
  BezierCurve(std::vector<vec3>& control_points);

  void setControlPoint(int i, vec3 point);
  Vertex evaluate(std::vector<vec3>& control_points, float t);
  Vertex evaluate(float t);
  Object generateObject(float resolution);
};

class BezierSurface {
 public:
  std::vector<std::vector<vec3>> control_points_m_;
  std::vector<std::vector<vec3>> control_points_n_;

  BezierSurface(int m, int n);
  BezierSurface(std::vector<std::vector<vec3>>& control_points);
  void setControlPoint(int i, int j, vec3 point);
  Vertex evaluate(std::vector<std::vector<vec3>>& control_points, float u, float v);
  Vertex evaluate(float u, float v);
  Object generateObject(float resolution);
};

void PatchCurve(std::vector<BezierCurve>& curves);
void PatchSurface(std::vector<std::vector<BezierSurface>>& surfaces);

#endif