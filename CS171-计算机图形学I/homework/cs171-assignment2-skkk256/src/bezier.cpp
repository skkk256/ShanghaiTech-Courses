#include <bezier.h>
#include <utils.h>
#include <vector>
#include <cmath>

BezierCurve::BezierCurve(int m) { control_points_.resize(m); }

BezierCurve::BezierCurve(std::vector<vec3>& control_points) {
  control_points_ = control_points;
}

void BezierCurve::setControlPoint(int i, vec3 point) {
  control_points_[i] = point;
}

/**
 * TODO: evaluate the point at t with the given control_points
 */
Vertex BezierCurve::evaluate(std::vector<vec3>& control_points, float t) {
  // !DELETE THIS WHEN YOU FINISH THIS FUNCTION
  std::vector<Vertex> temp_points(control_points.size());
  GLuint n = temp_points.size();
  vec3 temp_position{};

  for (int i = 0; i < n; ++i) {
      temp_points[i].position = control_points[i];
  }
  for (int k = 1; k < n; ++k) {
      for (int i = 0; i < n - k; ++i) {
          temp_position =  temp_points[i].position;
          temp_points[i].position = (1 - t) * temp_points[i].position + t * temp_points[i + 1].position;
//          temp_points[i].normal = glm::normalize(glm::cross(temp_points[i + 1].position - temp_points[i].position, vec3(0, 0, 1)));
          temp_points[i].normal = temp_points[i+1].position - temp_position; //it is the tangent
      }
  }

  return temp_points[0];
}

Vertex BezierCurve::evaluate(float t) {
  return evaluate(control_points_, t);
}

/**
 * TODO: generate an Object of the current Bezier curve
 */
Object BezierCurve::generateObject(float resolution) {
  Object obj;
  std::vector<Vertex> vertices;
  float step = 1.0f / resolution; // resolution is the number of points you want to generate on the curve
  for (float t = 0; t <= 1; t += step) {
    Vertex v = evaluate(t);
    vertices.push_back(v);
  }
  obj.setData(vertices);
  return obj;
}

BezierSurface::BezierSurface(int m, int n) {
  control_points_m_.resize(m);
  for (auto& sub_vec : control_points_m_) {
    sub_vec.resize(n);
  }
  control_points_n_.resize(n);
  for (auto& sub_vec : control_points_n_) {
    sub_vec.resize(m);
  }
}

BezierSurface::BezierSurface(std::vector<std::vector<vec3>>& control_points) {
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

/**
 * @param[in] i: index (i < m)
 * @param[in] j: index (j < n)
 * @param[in] point: the control point with index i, j
 */
void BezierSurface::setControlPoint(int i, int j, vec3 point) {
  control_points_m_[i][j] = point;
  control_points_n_[j][i] = point;
}

/**
 * TODO: evaluate the point at (u, v) with the given control points
 */
Vertex BezierSurface::evaluate(std::vector<std::vector<vec3>>& control_points,
                               float u, float v) {
  GLuint m = control_points.size();
  std::vector<vec3> temp_points(m);

  for (int i = 0; i < m; ++i) {
    BezierCurve curve(control_points[i]);
    temp_points[i] = (curve.evaluate(u).position);
  }

  BezierCurve final_curve(temp_points);
  return final_curve.evaluate(v);
}

Vertex BezierSurface::evaluate(float u, float v) {
  Vertex point_m = evaluate(control_points_m_, u, v);
  Vertex point_n = evaluate(control_points_n_, v, u);
  point_m.normal = glm::normalize(glm::cross(point_m.normal, point_n.normal));
  return point_m;
}

/**
 * TODO: generate an Object of the current Bezier surface
 */
Object BezierSurface::generateObject(float resolution) {
  Object obj;
  std::vector<Vertex> vertices{};
  std::vector<GLuint> indices;
  float step = 1.0 / resolution; // resolution is the number of points you want to generate on the surface in each direction
  for (float u = 0; u <= 1; u += step) {
    for (float v = 0; v <= 1; v += step) {
      Vertex vert = evaluate(u, v);
      vertices.push_back(vert);
    }
  }
  GLuint length = pow(vertices.size(), 0.5);

  for(int i = 0; i < length - 1; i++) {
    for(int j = 0; j < length - 1; j++) {
//      std::cout << i * length + j << " "
//                << i * length + j + 1 << " "
//                << (i + 1) * length + j << " "
//                << (i + 1) * length + j << " "
//                << i * length + j + 1 << " "
//                << (i + 1) * length + j + 1 << std::endl;
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

vec3 average_vec3(vec3 v1, vec3 v2) {
  return { (v1.x + v2.x) / 2, (v1.y + v2.y) / 2, (v1.z + v2.z) / 2 };
}

void PatchTwoSurface(BezierSurface &s1, BezierSurface &s2, const char direction) {
  GLuint m1 = s1.control_points_m_.size();
  GLuint n1 = s1.control_points_n_.size();
  GLuint m2 = s2.control_points_m_.size();
  GLuint n2 = s2.control_points_n_.size();
  assert(m1 > 1 && n1 > 1 && m2 > 1 && n2 > 1);
  switch (direction) {
    case 'm': // m1 == m2
      assert(m1 == m2);
      for (int i = 0; i < m1; i++) {
        s1.setControlPoint(i, n1-1, average_vec3(s1.control_points_m_[i][n1 - 2], s2.control_points_m_[i][1]));
        s2.setControlPoint(i, 0, average_vec3(s1.control_points_m_[i][n1 - 2], s2.control_points_m_[i][1]));
      }
      break;

    case 'n': // n1 == n2
      assert(n1 == n2);
      for (int i = 0; i < n1; i++) {
        s1.setControlPoint(m1-1, i, average_vec3(s1.control_points_m_[m1 - 2][i], s2.control_points_m_[1][i]));
        s2.setControlPoint(0, i, average_vec3(s1.control_points_m_[m1 - 2][i], s2.control_points_m_[1][i]));
      }
      break;

    default:
      throw std::runtime_error("Direction should be 'm' or 'n', which means along the direction that m1 == m2 or n1 == n2");
  }
}


void PatchCurve(std::vector<BezierCurve>& curves) {
  for (int i = 0; i < curves.size() - 1; i++) {
    GLuint index = curves[i].control_points_.size() - 1;
    if (curves[i].control_points_.size() < 2 || curves[i + 1].control_points_.size() < 2) {
      throw std::runtime_error("One point can't be a curve");
    }
    if (curves[i].control_points_[index] != curves[i + 1].control_points_[0]) {
      throw std::runtime_error("The control points of the curves are not connected");
    }
    curves[i].control_points_[index] = average_vec3(curves[i].control_points_[index-1], curves[i + 1].control_points_[1]);
    curves[i+1].control_points_[0] = curves[i].control_points_[index];
  }
}

void PatchSurface(std::vector<std::vector<BezierSurface>>& surfaces) {
  GLuint x = surfaces.size();
  GLuint y = surfaces[0].size();
  for (int i = 0; i < x; i++) {
    for (int j = 0; j < y - 1; j++) {
      PatchTwoSurface(surfaces[i][j], surfaces[i][j + 1], 'm');
    }
  }
  for (int j = 0; j < y; j++) {
    for (int i = 0; i < x - 1; i++) {
      PatchTwoSurface(surfaces[i][j], surfaces[i + 1][j], 'n');
    }
  }
}



void PatchSurface(std::vector<BezierSurface>& surfaces);
