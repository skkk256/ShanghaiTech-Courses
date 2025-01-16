#include <utils.h>
#include <vector>
#include <cmath>
#include "bspline.h"

BSplineCurve::BSplineCurve(std::vector<vec3>& control_points) {
  control_points_ = control_points;
  GLuint knot_num = control_points_.size() + degree + 1;
  for (int i = 0; i < degree; ++i)
    knot.push_back(0.0f);
  GLuint num = knot_num - 2 * degree;
  GLfloat delta = 1.0f / (float)(num - 1);
  for (int i = 0; i < num; ++i)
    knot.push_back(delta * i);
  for (int i = 0; i < degree; ++i)
    knot.push_back(1.0f);
}

vec3 BSplineCurve::evaluate(std::vector<vec3>& control_points, float t) {
  int k = degree;
  int n = control_points.size();
  int s = 0;

  if (t + 1e-3 > knot[n]) {
    k = n - 1;
  } else if (t + 1e-3 == knot[degree] ) {
    s = degree;
  }
  else {
    for (; k <= control_points.size(); ++k) {
      if (knot[k] <= t + 1e-3 && t + 1e-3 < knot[k + 1]) {
        break;
      }
    }
  }

  int h = degree - s;
  std::vector<vec3> temp_position(control_points.begin() + k - degree, control_points.begin() + k + 1);

  for (int r = 1; r <= h; ++r) {
    for (int i = k; i >= k - degree + r; --i) {
      int j = i - k + degree;
      float a = (t - knot[i]) / (knot[i + degree - r + 1] - knot[i]);
      temp_position[j] = (1.0f - a) * temp_position[j - 1] + a * temp_position[j];
    }
  }
  return temp_position[degree];
}

vec3 BSplineCurve::evaluate(float t) {
  return evaluate(control_points_, t);
}


BSplineSurface::BSplineSurface(std::vector<std::vector<vec3>>& control_points) {
  control_points_m_ = control_points;
  control_points_n_.resize(control_points[0].size());
  for (auto& sub_vec : control_points_n_) {
    sub_vec.resize(control_points.size());
  }
  for (int i = 0; i < control_points.size(); ++i) {
    for (int j = 0; j < control_points[0].size(); ++j) {
      control_points_n_[j][i] = control_points[i][j];
    }
  }
}

Vertex BSplineSurface::evaluate(std::vector<std::vector<vec3>>& control_points, float u, float v) {
  int m = control_points.size();

  std::vector<vec3> temp_points(m);
  for (int i = 0; i < m; ++i) {
    BSplineCurve curve(control_points[i]);
    temp_points[i] = curve.evaluate(u);
  }
  BSplineCurve final_curve(temp_points);
  Vertex vert;
  vert.position = final_curve.evaluate(v);
  vert.normal = glm::normalize(final_curve.evaluate(v + 1e-3) - vert.position);

  return vert;
}

Vertex BSplineSurface::evaluate(float u, float v) {
  Vertex vert;
  Vertex point_m = evaluate(control_points_m_, u, v);
  Vertex point_n = evaluate(control_points_n_, v, u);
  vert.position = (point_m.position + point_n.position) / 2.0f;
  vert.normal = glm::normalize(glm::cross(point_m.normal, point_n.normal));
  return vert;
}

Object BSplineSurface::generateObject(float resolution) {
  Object obj;
  std::vector<Vertex> vertices{};
  std::vector<GLuint> indices;

  float step = 1.0 / resolution; // resolution is the number of points you want to generate on the surface in each direction
  for (float u = 0; u <= 1; u += step) {
    for (float v = 0; v <= 1; v += step) {
      Vertex vert =  evaluate(u, v);
      vertices.push_back(vert);
    }
  }
  GLuint length = pow(vertices.size(), 0.5);

  for(int i = 0; i < length - 1; i++) {
    for(int j = 0; j < length - 1; j++) {
      indices.push_back(i * length + j);
      indices.push_back(i * length + j + 1);
      indices.push_back((i + 1) * length + j);
      indices.push_back((i + 1) * length + j);
      indices.push_back(i * length + j + 1);
      indices.push_back((i + 1) * length + j + 1);
    }
  }
  obj.setData(vertices, indices);
  return obj;
}