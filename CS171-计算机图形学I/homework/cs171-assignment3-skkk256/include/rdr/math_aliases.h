/**
 * @file math_aliases.h
 * @author ShanghaiTech CS171 TAs
 * @brief Creating aliases for math-based project is important, where the
 * interface should be aligned between projects (so we'll not use Eigen, whose
 * performance is not satisfying in small granularity) and easy-to-use. Aliases
 * are to be defined in a procedural way, i.e. `Dot(a, b)` instead of
 * `a.dot(b)`. If C++ 20 is available, define functions with concept. This file
 * is based on linalg.h https://github.com/sgorsten/linalg.
 * @version 0.1
 * @date 2023-04-12
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef __MATH_ALIASES_H__
#define __MATH_ALIASES_H__

#include <linalg.h>

#include <limits>
#include <sstream>

#include "rdr/platform.h"

RDR_NAMESPACE_BEGIN

/* ===================================================================== *
 *
 * Linear Algebra Primitives
 *
 * ===================================================================== */

template <typename T, int N>
using Vec = linalg::vec<T, N>;

template <typename T, int N, int M>
using Mat = linalg::mat<T, N, M>;

using Float  = float;
using Double = double;
using Vec2f  = Vec<Float, 2>;
using Vec2i  = Vec<int, 2>;
using Vec2u  = Vec<uint32_t, 2>;
using Vec2d  = Vec<Double, 2>;
using Vec3f  = Vec<Float, 3>;
using Vec3i  = Vec<int, 3>;
using Vec3u  = Vec<uint32_t, 3>;
using Vec3d  = Vec<Double, 3>;
using Vec4f  = Vec<Float, 4>;
using Vec4d  = Vec<Double, 4>;
using Mat3f  = Mat<Float, 3, 3>;
using Mat4f  = Mat<Float, 4, 4>;

// See https://en.cppreference.com/w/cpp/language/user_literal
constexpr Float operator"" _F(long double value) {
  return Float(static_cast<double>(value));
}

constexpr Float operator"" _F(unsigned long long int value) {
  return Float(static_cast<double>(value));
}

using namespace linalg::ostream_overloads;

template <typename T>
std::string ToString(const T &value) {
  std::ostringstream oss;
  oss << std::setprecision(4);
  oss << value;
  return oss.str();
}

/* ===================================================================== *
 *
 * Constants
 *
 * ===================================================================== */

constexpr Float Float_INF       = std::numeric_limits<Float>::infinity();
constexpr Float Float_MINUS_INF = -std::numeric_limits<Float>::infinity();
constexpr Float Float_EPSILON   = std::numeric_limits<Float>::epsilon();

constexpr Float RAY_DEFAULT_MIN = 1e-4;
constexpr Float RAY_DEFAULT_MAX = 1e7;
constexpr Float PI              = 3.14159265358979323846;
constexpr Float INV_PI          = 0.31830988618379067154;
constexpr Float EPS             = 1e-4;
constexpr Float NORMAL_EPS      = 1e-6;

constexpr Mat4f IdentityMatrix4 = {
    {1, 0, 0, 0},
    {0, 1, 0, 0},
    {0, 0, 1, 0},
    {0, 0, 0, 1}
};

/* ===================================================================== *
 *
 * Linear Algebra Operations
 *
 * ===================================================================== */

template <typename T>
struct vec_type;

template <typename T, int M>
struct vec_type<linalg::vec<T, M>> {
  using value_type                  = T;
  static constexpr std::size_t size = M;
};

template <typename T>
struct is_vec_type : std::false_type {};

template <typename T, int M>
struct is_vec_type<linalg::vec<T, M>> : std::true_type {};

template <typename T>
inline constexpr bool is_vec_type_v = is_vec_type<T>::value;

template <typename T, typename U>
RDR_FORCEINLINE constexpr Vec<T, vec_type<U>::size> Cast(const U &v) {
  Vec<T, vec_type<U>::size> result;
  for (std::size_t i = 0; i < vec_type<U>::size; ++i)
    result[i] = static_cast<T>(v[i]);
  return result;
}

template <typename T>
RDR_FORCEINLINE decltype(auto) Dot(const T &x, const T &y) {
  return linalg::dot(x, y);
}

template <typename M, typename V>
RDR_FORCEINLINE decltype(auto) Mul(const M &x, const V &y) {
  return linalg::mul(x, y);
}

template <typename M>
RDR_FORCEINLINE decltype(auto) Inverse(const M &x) {
  return linalg::inverse(x);
}

template <typename M>
RDR_FORCEINLINE decltype(auto) Transpose(const M &x) {
  return linalg::transpose(x);
}

template <typename T>
RDR_FORCEINLINE decltype(auto) SquareNorm(const T &x) {
  return Dot(x, x);
}

template <typename T>
RDR_FORCEINLINE decltype(auto) Norm(const T &x) {
  return std::sqrt(SquareNorm(x));
}

template <typename T>
RDR_FORCEINLINE decltype(auto) Cross(const T &x, const T &y) {
  return linalg::cross(x, y);
}

template <typename T>
RDR_FORCEINLINE decltype(auto) Normalize(const T &x) {
  return x / T{Norm(x)}; /* cannot perform implicitly broadcast */
}

template <typename T>
RDR_FORCEINLINE decltype(auto) Max(const T &x, const T &y) {
  return linalg::max(x, y);
}

template <typename T, typename... Args>
RDR_FORCEINLINE decltype(auto) Max(const T &x, const Args &...args) {
  return Max(x, Max(args...));
}

template <typename T>
RDR_FORCEINLINE decltype(auto) Min(const T &x, const T &y) {
  return linalg::min(x, y);
}

template <typename T>
RDR_FORCEINLINE decltype(auto) Pow(const T &x, Float y) {
  return linalg::pow(x, y);
}

template <typename T, typename... Args>
RDR_FORCEINLINE decltype(auto) Min(const T &x, const Args &...args) {
  return Min(x, Min(args...));
}

template <std::size_t _>
RDR_FORCEINLINE decltype(auto) Any(const Vec<bool, _> &x) {
  return linalg::any(x);
}

template <std::size_t _>
RDR_FORCEINLINE decltype(auto) All(const Vec<bool, _> &x) {
  return linalg::all(x);
}

template <typename T>
RDR_FORCEINLINE decltype(auto) ReduceSum(const T &x) {
  return linalg::sum(x);
}

template <typename T>
RDR_FORCEINLINE decltype(auto) ReduceProduct(const T &x) {
  return linalg::product(x);
}

template <typename T>
RDR_FORCEINLINE decltype(auto) ReduceMin(const T &x) {
  return linalg::minelem(x);
}

template <typename T>
RDR_FORCEINLINE decltype(auto) ReduceMax(const T &x) {
  return linalg::maxelem(x);
}

template <typename T>
RDR_FORCEINLINE int ArgMin(const T &x) {
  return linalg::argmin(x);
}

template <typename T>
RDR_FORCEINLINE int ArgMax(const T &x) {
  return linalg::argmax(x);
}

template <typename T>
RDR_FORCEINLINE decltype(auto) AllClose(
    const T &x, const T &y, const Float &eps = EPS) {
  bool result = true;
  for (std::size_t i = 0; i < vec_type<T>::size; ++i)
    result &= std::abs(x[i] - y[i]) < eps;
  return result;
}

template <typename T>
RDR_FORCEINLINE decltype(auto) GetGB(const T &x) {
  return static_cast<double>(x) / 1024 / 1024 / 1024;
}

template <typename T>
RDR_FORCEINLINE decltype(auto) GetMB(const T &x) {
  return static_cast<double>(x) / 1024 / 1024;
}

template <typename T>
RDR_FORCEINLINE decltype(auto) GetKB(const T &x) {
  return static_cast<double>(x) / 1024;
}

template <typename T>
RDR_FORCEINLINE decltype(auto) Sign(const T &x) {
  // For numeric reason, we do not follow the standard sign function
  return x >= 0 ? 1 : -1;
}

RDR_FORCEINLINE uint32_t Next2Pow(uint32_t n) {
  double n_  = log2(static_cast<double>(n));
  uint32_t k = static_cast<uint32_t>(ceil(n_));
  return pow(2, k);
}

/* ===================================================================== *
 *
 * Re-definition of some math functions
 *
 * ===================================================================== */

RDR_FORCEINLINE float sqrt(const float &x) {
  return sqrtf(x);
}

RDR_FORCEINLINE double sqrt(const double &x) {
  return std::sqrt(x);
}

RDR_FORCEINLINE float abs(const float &x) {
  return fabsf(x);
}

RDR_FORCEINLINE double abs(const double &x) {
  return std::abs(x);
}

RDR_FORCEINLINE float max(const float &x, const float &y) {
  return fmaxf(x, y);
}

RDR_FORCEINLINE double max(const double &x, const double &y) {
  return std::max(x, y);
}

RDR_FORCEINLINE float min(const float &x, const float &y) {
  return fminf(x, y);
}

RDR_FORCEINLINE double min(const double &x, const double &y) {
  return std::min(x, y);
}

RDR_NAMESPACE_END

/* ===================================================================== *
 *
 * fmtlib-related definitions
 * *must be globally visible*
 *
 * ===================================================================== */

template <typename T, int N>
struct fmt::formatter<RDR_NAMESPACE_NAME::Vec<T, N>> : formatter<std::string> {
  template <typename FormatContext>
  auto format(
      const RDR_NAMESPACE_NAME::Vec<T, N> &c, FormatContext &ctx) const {
    const std::string name = RDR_NAMESPACE_NAME::ToString(c);
    return formatter<std::string>::format(name, ctx);
  }
};

template <typename T, int N, int M>
struct fmt::formatter<RDR_NAMESPACE_NAME::Mat<T, N, M>>
    : formatter<std::string> {
  template <typename FormatContext>
  auto format(
      const RDR_NAMESPACE_NAME::Mat<T, N, M> &c, FormatContext &ctx) const {
    const std::string name = RDR_NAMESPACE_NAME::ToString(c);
    return formatter<std::string>::format(name, ctx);
  }
};

#endif
