/**
 * @file conventions.cpp
 * @author ShanghaiTech CS171 TAs
 * @brief This file intends to familar you with those basic components in this
 * renderer, as well as those math conventions. Everyone SHOULD carefully go
 * through this file.
 * @version 0.1
 * @date 2023-04-12
 *
 * @copyright Copyright (c) 2023
 *
 */
#include <numeric>

#include "rdr/rdr.h"

using namespace RDR_NAMESPACE_NAME;

int main() {
  // Handedness Convention
  Vec3f x(1, 0, 0);
  Vec3f y(0, 1, 0);
  auto z = Cross(x, y);  // no matter

  std::string text1 = R"(
// The renderer is a right-handed system
// You can imagine the scene as x pointing to the right, y pointing up, and z
// pointing towards you(if you look forward), which respect the right-hand rule. 
// Note that this is for global coordinate, i.e. the scene setting. 
)";
  // Just a test, consistent under any coordinate system.
  print("{}// [1] z = Cross(x, y) = {}\n", text1, z);

  auto text2 = R"(
// Meanwhile the local coordinate system of hand is right (by convention). Where
// *z* is the principal axis pointing up. WHICH IS DIFFERENT FROM THE GLOBAL COORDINATE.
// This is weird, but it will make spherical coordinates easier to understand. And as 
// long as the implementation of coordinate transition is correct, it does not bring 
// much complexity. For example,
)";
  Vec3f direction(1, 0, 1);
  direction = Normalize(direction);
  print("{}// [2] direction = {}\n", text2, direction);
  print("// [3] cosTheta = Dot(direction, y) = {}\n", Dot(direction, z));
  print("// [4] theta = acos(cosTheta) = {} = pi/4\n", acos(Dot(direction, z)));

  auto text3 = R"(
// For spherical coordinates, we uses theta for vertical index, and phi for
// horizontal index. That is to say, 0 <= theta < pi and 0 <= phi < 2pi.
)";
  print("{}// [5] SphericalDirection(theta=0, phi=0)   = {}\n", text3,
      SphericalDirection(0, 0));
  print("// [6] SphericalDirection(theta=pi/2, 0)    = {}\n",
      SphericalDirection(PI / 2, 0));
  auto vec_ = SphericalDirection(PI / 2, PI / 4);
  print("// [7] SphericalDirection(theta=pi/2, pi/4) = {}\n", vec_);
  print("// [8] InverseSphericalDirection({}) = {}\n", vec_,
      InverseSphericalDirection(vec_));

  Vec3f n(0, 0, 1), x_, y_;
  CoordinateSystemFromNormal(n, x_, y_);
  auto text4 = R"(
// Let's examine the coordinate transition system, which will be heavily used
// in all components. We build a coordinate system:
)";
  print("{}// [9] x_ = {}, y_ = {}, Cross(x_, y_) = {}\n", text4, x_, y_,
      Cross(x_, y_));

  // We wrap this mechanism with frame.
  Frame frame(Vec3f(0, 0, 1));
  Vec3f gvec(0, 1, 0);
  auto lvec = frame.WorldToLocal(gvec);
  auto rvec = frame.LocalToWorld(lvec);
  print("// [10] original_vec = {}, restored_vec = {}\n", gvec, rvec);

  // Now that the basic use of math library is introduced, you can goto practice
  // yourself! (the second task)
}