#include "rdr/mipmap.h"

#include <algorithm>
#include <cstdint>
#include <exception>

#include "rdr/math_aliases.h"
#include "rdr/platform.h"

RDR_NAMESPACE_BEGIN

Float renderer::MIPMap::gs_weight[renderer::MIPMap::WeightSize] = {};

MIPMap::MIPMap(const Vec2u &in_resolution, const vector<Float> &in_data,
    LookUpMethod in_method, ImageWrap in_wrap_mode)
    : method(in_method), wrap_mode(in_wrap_mode) {
  Vec2u res(1, 1);
  for (; res[0] < in_resolution[0]; res[0] <<= 1)
    ;
  for (; res[1] < in_resolution[1]; res[1] <<= 1)
    ;

  const Float *pre_data = in_data.data();
  Vec2u pre_res         = in_resolution;

  // Reverse the memory, or the pointer of data
  // will change when you push_back
  data.reserve(res[0] * res[1] * 6);

  uint32_t cur_offset = 0;
  while (res[0] != 1 || res[1] != 1) {
    offset.push_back(cur_offset);

    const auto &cur = resolution.emplace_back(res);

    for (uint32_t y = 0; y < cur[1]; y++)
      for (uint32_t x = 0; x < cur[0]; x++) {
        const auto texel =
            Interpolate(pre_data, pre_res, Vec2f(x + 0.5f, y + 0.5f) / cur);
        for (uint32_t index = 0; index < 3; index++)
          data.push_back(texel[index]);
        // aligned for alpha
        data.push_back(0);
      }

    cur_offset += cur[0] * cur[1];
    pre_data = data.data() + 4 * offset.back();
    pre_res  = cur;

    res[0] = std::max(1u, res[0] >> 1);
    res[1] = std::max(1u, res[1] >> 1);
  }

  // Initialize EWA filter weights (2D Gaussian distribution) if needed
  if (gs_weight[0] == 0.) {
    for (int i = 0; i < WeightSize; ++i) {
      Float alpha  = 2;
      Float r2     = Float(i) / Float(WeightSize - 1);
      gs_weight[i] = std::exp(-alpha * r2) - std::exp(-alpha);
    }
  }
}

Vec3f MIPMap::Texel(uint32_t l, uint32_t s, uint32_t t) const {
  if (l >= Level()) {
    Exception_("the texel level {} is out of bound {}", l, Level());
    return {0, 0, 0};
  }
  const auto &width  = resolution[l][0];
  const auto &height = resolution[l][1];
  const auto &ofs    = offset[l];
  switch (wrap_mode) {
    case ImageWrap::Repeat:
      s %= width;
      t %= height;
      break;
    case ImageWrap::Clamp:
      s = std::clamp(s, 0u, width - 1);
      t = std::clamp(t, 0u, height - 1);
      break;
    case ImageWrap::Black:
      if (s < 0 || s >= width || t < 0 || t >= height) return {0, 0, 0};
  }
  return Vec3f(&data[4 * (ofs + t * width + s)]);
}

Vec3f MIPMap::LookUp(const Vec2f &st, Float width) const noexcept {
  Float l = Level() - 1 + std::log2(std::max(width, (Float)1e-8));
  if (l < 0)
    return TriTexel(0, st);
  else if (l >= Level())
    return TriTexel(Level() - 1, st);
  else {
    uint32_t L = std::floor(l), R = L + 1;
    return (R - l) * TriTexel(L, st) + (l - L) * TriTexel(R, st);
  }
}

Vec3f MIPMap::LookUp(const Vec2f &st, Vec2f dstdx, Vec2f dstdy) const noexcept {
  switch (method) {
    case LookUpMethod::TriLinearInterpolation: {
      Float width = std::max(std::max(std::abs(dstdx[0]), std::abs(dstdx[1])),
          std::max(std::abs(dstdy[0]), std::abs(dstdy[1])));
      return LookUp(st, width);
    }
    case LookUpMethod::EWA: {
      if (SquareNorm(dstdx) < SquareNorm(dstdy)) std::swap(dstdx, dstdy);
      Float major = Norm(dstdx);
      Float minor = Norm(dstdy);

      // Clamp ellipse eccentricity if too large
      // Slender ellipse will significantly reduce performance
      if (minor * maxAnisotropy < major && minor > 0) {
        Float scale = major / (minor * maxAnisotropy);
        dstdy *= scale;
        minor *= scale;
      }
      if (minor == 0) return TriTexel(0, st);

      Float l = std::max(0.f, Level() - 1.f + std::log2(minor));

      uint32_t L = std::floor(l), R = L + 1;
      return (R - l) * EWA(L, st, dstdx, dstdy) +
             (l - L) * EWA(R, st, dstdx, dstdy);
    }
    default:
      return {0, 0, 0};
  }
}

Vec3f MIPMap::TriTexel(uint32_t l, const Vec2f &st) const noexcept {
  l = std::clamp(l, 0u, Level() - 1);

  const Float s = std::max(st[0] * resolution[l][0] - 0.5f, 0.0f);
  const Float t = std::max(st[1] * resolution[l][1] - 0.5f, 0.0f);

  const uint32_t lo_s = std::floor(s), hi_s = lo_s + 1;
  const uint32_t lo_t = std::floor(t), hi_t = lo_t + 1;

  return (hi_s - s) * (hi_t - t) * Texel(l, lo_s, lo_t) +
         (hi_s - s) * (t - lo_t) * Texel(l, lo_s, hi_t) +
         (s - lo_s) * (hi_t - t) * Texel(l, hi_s, lo_t) +
         (s - lo_s) * (t - lo_t) * Texel(l, hi_s, hi_t);
}

Vec3f MIPMap::EWA(uint32_t l, const Vec2f &st, const Vec2f &dst0,
    const Vec2f &dst1) const noexcept {
  l = std::clamp(l, 0u, Level() - 1);

  const Float s = std::max(st[0] * resolution[l][0] - 0.5f, 0.0f);
  const Float t = std::max(st[1] * resolution[l][1] - 0.5f, 0.0f);

  const auto sdst0 = dst0 * resolution[l];
  const auto sdst1 = dst1 * resolution[l];

  Float A = sdst0[1] * sdst0[1] + sdst1[1] * sdst1[1] + 1;
  Float B = -2 * (sdst0[0] * sdst0[1] + sdst1[0] * sdst1[1]);
  Float C = sdst0[0] * sdst0[0] + sdst1[0] * sdst1[0] + 1;

  const Float invF = 1 / (A * C - B * B * 0.25f);

  A *= invF;
  B *= invF;
  C *= invF;

  // Bounding box of ellipse
  Float det    = -B * B + 4 * A * C;
  Float invDet = 1 / det;
  Float uSqrt = std::sqrt(det * C), vSqrt = std::sqrt(A * det);
  int s0 = std::ceil(st[0] - 2 * invDet * uSqrt);
  int s1 = std::floor(st[0] + 2 * invDet * uSqrt);
  int t0 = std::ceil(st[1] - 2 * invDet * vSqrt);
  int t1 = std::floor(st[1] + 2 * invDet * vSqrt);

  Vec3f sum{0, 0, 0};
  Float sumWts = 0;
  for (int it = t0; it <= t1; ++it) {
    Float tt = it - st[1];
    for (int is = s0; is <= s1; ++is) {
      Float ss = is - st[0];
      // Compute squared radius and filter texel if inside ellipse
      Float r2 = A * ss * ss + B * ss * tt + C * tt * tt;
      if (r2 < 1) {
        uint32_t index =
            std::min(static_cast<uint32_t>(r2 * WeightSize), WeightSize - 1);
        Float weight = gs_weight[index];
        sum += Texel(l, is, it) * weight;
        sumWts += weight;
      }
    }
  }

  return sum / sumWts;
}

RDR_NAMESPACE_END