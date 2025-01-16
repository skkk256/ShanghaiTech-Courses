/**
 * @file math_utils.h
 * @author ShanghaiTech CS171 TAs
 * @brief
 * @version 0.1
 * @date 2023-04-13
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef __MATH_UTILS_H__
#define __MATH_UTILS_H__

#include <array>
#include <random>

#include "rdr/canary.h"
#include "rdr/math_aliases.h"
#include "rdr/platform.h"

#include <iostream>

RDR_NAMESPACE_BEGIN

/* ===================================================================== *
 *
 * Math Utils
 *
 * ===================================================================== */

RDR_FORCEINLINE Float CosTheta(const Vec3f &w) {
  return w.z;
}

RDR_FORCEINLINE Float Cos2Theta(const Vec3f &w) {
  return w.z * w.z;
}

RDR_FORCEINLINE Float AbsCosTheta(const Vec3f &w) {
  return std::abs(w.z);
}

RDR_FORCEINLINE Float Sin2Theta(const Vec3f &w) {
  return std::max((Float)0, (Float)1 - Cos2Theta(w));
}

RDR_FORCEINLINE Float SinTheta(const Vec3f &w) {
  return std::sqrt(Sin2Theta(w));
}

RDR_FORCEINLINE Float TanTheta(const Vec3f &w) {
  return SinTheta(w) / CosTheta(w);
}

RDR_FORCEINLINE Float Tan2Theta(const Vec3f &w) {
  return Sin2Theta(w) / Cos2Theta(w);
}

RDR_FORCEINLINE Float CosPhi(const Vec3f &w) {
  Float sin_theta = SinTheta(w);
  return (sin_theta == 0) ? 1 : std::clamp<Float>(w.x / sin_theta, -1, 1);
}

RDR_FORCEINLINE Float SinPhi(const Vec3f &w) {
  Float sin_theta = SinTheta(w);
  return (sin_theta == 0) ? 0 : std::clamp<Float>(w.y / sin_theta, -1, 1);
}

RDR_FORCEINLINE Float Cos2Phi(const Vec3f &w) {
  return CosPhi(w) * CosPhi(w);
}

RDR_FORCEINLINE Float Sin2Phi(const Vec3f &w) {
  return SinPhi(w) * SinPhi(w);
}

RDR_FORCEINLINE Vec3f Reflect(const Vec3f &wo, const Vec3f &n) {
  return -wo + 2 * Dot(wo, n) * n;
}

RDR_FORCEINLINE Vec3f FaceForward(const Vec3f &target, const Vec3f &w) {
  return (Dot(target, w) < 0) ? -w : w;
}

/**
 * @brief Refract a ray with incident direction wi and surface normal n.
 * @param eta The ratio of eta between incident medium and transmitted medium.
 * @return true if the ray is refracted, false if the ray is totally reflected.
 */
RDR_FORCEINLINE bool Refract(
    const Vec3f &wi, const Vec3f &n, Float eta, Vec3f &wt) {
  AssertAllValid(wi, n, eta, wt);
  Float cos_theta_I  = Dot(n, wi);
  Float sin2_theta_I = std::max(0.f, 1.f - cos_theta_I * cos_theta_I);
  Float sin2_theta_T = eta * eta * sin2_theta_I;
  if (sin2_theta_T >= 1) return false;
  Float cos_theta_T = std::sqrt(1 - sin2_theta_T);

  // Write the output
  wt = eta * -wi + (eta * cos_theta_I - cos_theta_T) * Vec3f(n);
  AssertAllValid(wt);
  AssertAllNormalized(wt);
  return true;
}

RDR_FORCEINLINE bool SameHemisphere(const Vec3f &w, const Vec3f &wp) {
  return w.z * wp.z > 0;
}

RDR_FORCEINLINE Vec3f SphericalDirection(
    Float sin_theta, Float cos_theta, Float phi) {
  return {sin_theta * std::cos(phi), sin_theta * std::sin(phi), cos_theta};
}

RDR_FORCEINLINE Vec3f SphericalDirection(Float theta, Float phi) {
  return SphericalDirection(std::sin(theta), std::cos(theta), phi);
}

RDR_FORCEINLINE Vec3f SphericalDirection(const Vec2f &scoord) {
  return SphericalDirection(scoord[0], scoord[1]);
}

RDR_FORCEINLINE Vec2f InverseSphericalDirection(const Vec3f &w) {
  const auto w_ = Normalize(w);
  Float phi     = std::atan2(w_.y, w_.x);
  if (phi < 0) phi += 2 * PI;
  return {std::acos(w_.z), phi};
}

RDR_FORCEINLINE void CoordinateSystemFromNormal(
    const Vec3f &in_n, Vec3f &x, Vec3f &y) {
  // Don't need to understand.
  // Or you can ask ChatGPT
  using InternalScalarType = Double;
  using InternalVecType    = Vec<InternalScalarType, 3>;

  const InternalVecType n = Cast<InternalScalarType>(in_n);
  if (std::abs(n.x) > std::abs(n.y)) {
    InternalScalarType invLen = 1.0f / std::sqrt(n.x * n.x + n.z * n.z);

    y = Vec3f(n.z * invLen, 0.0f, -n.x * invLen);
  } else {
    InternalScalarType invLen = 1.0f / std::sqrt(n.y * n.y + n.z * n.z);

    y = Vec3f(0.0f, n.z * invLen, -n.y * invLen);
  }

  x = cross(y, in_n);
  AssertAllValid(x, y);
}

constexpr Vec3f DefaultFrameLocalNormal = Vec3f(0, 0, 1);

struct Frame {
  Frame() : n(DefaultFrameLocalNormal) { CoordinateSystemFromNormal(n, x, y); }
  Frame(const Vec3f &n) : n(n) { CoordinateSystemFromNormal(n, x, y); }

  Vec3f WorldToLocal(const Vec3f &v /* world */) const {
    AssertAllValid(v);
    return Normalize(Vec3f(Dot(v, x), Dot(v, y), Dot(v, n)));
  }

  Vec3f LocalToWorld(const Vec3f &v /* local */) const {
    AssertAllValid(v);
    return Normalize(v.x * x + v.y * y + v.z * n);
  }

  Vec3f n, x, y;
};

/// Definitions for OffsetRayOrigin
namespace detail_ {
RDR_FORCEINLINE float __host_int_as_float(int a) {
  union {
    int m_a;
    float m_b;
  } u;
  u.m_a = a;
  return u.m_b;
}
RDR_FORCEINLINE int __host_float_as_int(float b) {
  union {
    int m_a;
    float m_b;
  } u;
  u.m_b = b;
  return u.m_a;
}
}  // namespace detail_

/**
 * @brief Offset the ray to avoid self-intersection
 * @note This implementation is from
 * https://research.nvidia.com/publication/2019-03_fast-and-robust-method-avoiding-self-intersection,
 * which provides a way to adaptively construct the new position without
 * tweaking the SHADOW_NORMAL param.
 * The observation is that, using a fixed EPS is not scene invariant and scale
 * invariant. We know that floating-point arithmetics's *relative accuracy*
 * remains almost invariant, but its *absolute accuracy* does not. So a scene's
 * EPS with an characteristic size of 10 is absolutely different from a size of
 * 1e9. That is, the absolute accuracy of intersecting a distant triangle is
 * much more lower. So int arithmetics is used, in which the absolute accuracy
 * is maintained.
 *
 * @param p The ray's original position
 * @param n The *geometry* normal
 * @param dir The ray's direction
 * @return Vec3f representing the ray's original position after offset
 */
RDR_FORCEINLINE Vec3f OffsetRayOrigin(const Vec3f &p, const Vec3f &n) {
  // Definition of integer arithmetics functions
  const auto &int_as_float = detail_::__host_int_as_float;
  const auto &float_as_int = detail_::__host_float_as_int;
  if constexpr (std::is_same_v<Float, float>) {
    constexpr float origin      = 1 / 32.0F;
    constexpr float float_scale = 1 / 65536.0F;
    constexpr float int_scale   = 256.0F;
    const Vec3f &offset         = n;
    const Vec3i &offset_int     = Cast<int>(offset * int_scale);
    const Vec3f &p_int{int_as_float(float_as_int(p.x) +
                                    ((p.x < 0) ? -offset_int.x : offset_int.x)),
        int_as_float(
            float_as_int(p.y) + ((p.y < 0) ? -offset_int.y : offset_int.y)),
        int_as_float(
            float_as_int(p.z) + ((p.z < 0) ? -offset_int.z : offset_int.z))};
    return {fabsf(p.x) < origin ? p.x + float_scale * offset.x : p_int.x,
        fabsf(p.y) < origin ? p.y + float_scale * offset.y : p_int.y,
        fabsf(p.z) < origin ? p.z + float_scale * offset.z : p_int.z};
  } else {
    return p + n * EPS;
  }
}

/// [0, 1)
RDR_FORCEINLINE Float Clamp01(Float v) {
  return std::clamp<Float>(v, 0, 1 - Float_EPSILON);
}

template <typename T>
RDR_FORCEINLINE T Mod(T a, T b) {
  if constexpr (std::is_same_v<T, Float>) {
    return std::fmod(a, b);
  } else {
    T result = a - (a / b) * b;
    return (T)((result < 0) ? result + b : result);
  }
}

RDR_FORCEINLINE uint8_t GammaCorrection(float radiance) {
  /// This with result in different result from mitsuba 0.6, whose tone mapper
  /// is really complex. So if you want to precisely debug the renderer, use
  /// exr.
  return static_cast<uint8_t>(255.0F * Clamp01(powf(radiance, 1.F / 2.2F)));
}

RDR_FORCEINLINE Float Radians(Float x) {
  return x * PI / 180;
}

RDR_FORCEINLINE Vec3f DeNan(const Vec3f &vec, float val) {
  Vec3f tmp = vec;
  if (vec.x != vec.x) tmp.x = val;
  if (vec.y != vec.y) tmp.y = val;
  if (vec.z != vec.z) tmp.z = val;
  return tmp;
}

/* ===================================================================== *
 *
 * Sampling-related Utils
 *
 * ===================================================================== */

class Sampler {
public:
  virtual ~Sampler() = default;

  RDR_FORCEINLINE virtual bool resetAfterIteration() { return true; }
  RDR_FORCEINLINE virtual Float get1D() { return Clamp01(dis(engine)); }
  RDR_FORCEINLINE virtual Vec2f get2D() { return {get1D(), get1D()}; }
  RDR_FORCEINLINE virtual void setSeed(int i) { engine.seed(i); }
  RDR_FORCEINLINE virtual void setPixelIndex2D(const Vec2i &index) {
    pixel_index = index;
  }

  RDR_FORCEINLINE virtual const Vec2i &getPixelIndex2D() const {
    return pixel_index;
  }

  RDR_FORCEINLINE virtual Vec2f getPixelSample() {
    return Cast<Float>(pixel_index) + get2D();
  }

  /** Shuffle a given array using the engine */
  template <typename InIterator>
  void shuffle(InIterator begin, InIterator end) {
    std::shuffle(begin, end, engine);
  }

protected:
  Vec2i pixel_index;
  std::mt19937 engine;
  std::uniform_real_distribution<Float> dis{0, 1 - Float_EPSILON};
};

/**
 * @brief Different measure of samples. For example, sampling the triangle
 * produces Area measure. Sampling the hemisphere produces SolidAngle measure.
 * PDF accross different measure should be converted respectively.
 */
enum class EMeasure {
  EUnknownMeasure = 0,
  ESolidAngle,
  EArea,
};

RDR_FORCEINLINE Vec2f UniformSampleDisk(const Vec2f &u) {
  // TODO: fill in your implementation here.
  // Derive the formula of uniformly sampling a point on a disk with radius one.
//  std::random_device rd;  // 用于获取随机数的种子
//  std::mt19937 gen(rd()); // 使用Mersenne Twister算法生成随机数
//  std::uniform_real_distribution<> dis(0.0, 1.0); // 定义分布范围
//  Float a = dis(gen);
//  Float b = dis(gen);
  Float a = u.x;
  Float b = 2 * PI * u.y;
  Float x = std::sqrt(a) * std::cos(b);
  Float y = std::sqrt(a) * std::sin(b);

  return {x, y};
}

RDR_FORCEINLINE Vec3f UniformSampleHemisphere(const Vec2f &u) {
  // TODO0: fill in your implementation here.
  // You can use the `SphericalDirection()` as a helper.
  Float theta = std::acos(u.x);
  Float phi = 2 * PI * u.y;
  return SphericalDirection(theta, phi);
}

RDR_FORCEINLINE Vec3f CosineSampleHemisphere(const Vec2f &u) {
  // We want p(w) = C*cos(theta), and we sample from (theta, phi).
  // Since dw = sin(theta) dtheta dphi, we have sin(theta) p(w) = p(theta,
  // phi) sin(theta)*C*cos(theta) = p(theta | phi) p(phi) = p(theta |
  // phi)/(2pi) p(theta | phi) = pi*C*sin(2*theta) -> p(theta | phi) =
  // sin(theta)^2, where 0 <= theta < pi/2. P^{-1}(theta, phi) =
  // arccos(sqrt(Uniform(0, 1))). Then
  //   theta ~ arccos(sqrt(Uniform(0, 1)))
  //   phi ~ Uniform(0, 2pi)
  Float theta = std::acos(std::sqrt(u.x));
  Float phi   = u.y * 2 * PI;
  return SphericalDirection(theta, phi);
}

RDR_FORCEINLINE Vec3f UniformSampleSphere(const Vec2f &u) {
  // TODO: fill in your implementation here.
  // You can use the `SphericalDirection()` as a helper.
  Float theta = std::acos(1 - 2 * u.x);
  Float phi = 2 * PI * u.y;
  return SphericalDirection(theta, phi);
}

RDR_FORCEINLINE Vec3f UniformSampleTriangle(const Vec2f &u) {
  Float su0 = std::sqrt(u.x);
  Float b0  = 1 - su0;
  Float b1  = u.y * su0;
  return {b0, b1, 1.0F - b0 - b1};
}

/*
    pbrt source code is Copyright(c) 1998-2016
                        Matt Pharr, Greg Humphreys, and Wenzel Jakob.

    This file is part of pbrt.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

    - Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    - Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
    IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
    TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
    PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */
struct Distribution1D {
  // Distribution1D Public Methods
  Distribution1D(const Float *f, int n) : func(f, f + n), cdf(n + 1) {
    // Compute integral of step function at $x_i$
    cdf[0] = 0;
    for (int i = 1; i < n + 1; ++i) cdf[i] = cdf[i - 1] + func[i - 1] / n;

    // Transform step function integral into CDF
    funcInt = cdf[n];
    if (funcInt == 0) {
      for (int i = 1; i < n + 1; ++i) cdf[i] = Float(i) / Float(n);
    } else {
      for (int i = 1; i < n + 1; ++i) cdf[i] /= funcInt;
    }
  }
  int size() const { return (int)func.size(); }
  Float sampleContinuous(Float u, Float *pdf, int *off = nullptr) const {
    // Find surrounding CDF segments and _offset_
    int offset = FindInterval(
        (int)cdf.size(), [&](int index) { return cdf[index] <= u; });
    if (off != nullptr) *off = offset;
    // Compute offset along CDF segment
    Float du = u - cdf[offset];
    if ((cdf[offset + 1] - cdf[offset]) > 0) {
      assert(cdf[offset + 1] > cdf[offset]);
      du /= (cdf[offset + 1] - cdf[offset]);
    }
    IsAllValid(du);

    // Compute PDF for sampled offset
    if (pdf) *pdf = (funcInt > 0) ? func[offset] / funcInt : 0;

    // Return $x\in{}[0,1)$ corresponding to sample
    return (offset + du) / size();
  }
  int sampleDiscrete(
      Float u, Float *pdf = nullptr, Float *uRemapped = nullptr) const {
    // Find surrounding CDF segments and _offset_
    int offset = FindInterval(
        (int)cdf.size(), [&](int index) { return cdf[index] <= u; });
    if (pdf) *pdf = (funcInt > 0) ? func[offset] / (funcInt * size()) : 0;
    if (uRemapped)
      *uRemapped = (u - cdf[offset]) / (cdf[offset + 1] - cdf[offset]);
    if (uRemapped) assert(*uRemapped >= 0.f && *uRemapped <= 1.f);
    return offset;
  }
  Float discretePDF(int index) const {
    assert(index >= 0 && index < size());
    return func[index] / (funcInt * size());
  }
  Float getIntegral() const { return funcInt; }

  // Distribution1D Public Data
  std::vector<Float> func, cdf;
  Float funcInt;

private:
  template <typename Predicate>
  int FindInterval(int size, const Predicate &pred) const {
    int first = 0, len = size;
    while (len > 0) {
      int half = len >> 1, middle = first + half;
      // Bisect range based on value of _pred_ at _middle_
      if (pred(middle)) {
        first = middle + 1;
        len -= half + 1;
      } else
        len = half;
    }
    return std::clamp<int>(first - 1, 0, size - 2);
  }
};

/* ===================================================================== *
 *
 * Photon-mapping related Kernels
 *
 * ===================================================================== */

RDR_FORCEINLINE Float RawEpanechnikovKernel(Float distance, Float radius) {
  Float u = distance / radius;
  return max(1 - u * u, 0);
}

RDR_FORCEINLINE Float EpanechnikovKernel2D(Float distance, Float radius) {
  Float u = distance / radius;
  return max(1 - u * u, 0) * (2 * INV_PI) / (radius * radius);
}

RDR_NAMESPACE_END

#endif
