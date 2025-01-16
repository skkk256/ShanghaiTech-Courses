/**
 * @file mipmap.h
 * @author ShanghaiTech CS171 TAs
 * @brief MIPMap is a pre-allocated with additional storage space data struct.
 * It is designed for improving image texture quality by reducing aliasing and
 * Moiré patterns that occur at large viewing distances.
 * @version 0.1
 * @date 2023-07-25
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef __MIPMAP_H__
#define __MIPMAP_H__

#include <cstdint>

#include "rdr/platform.h"
#include "rdr/rdr.h"

RDR_NAMESPACE_BEGIN

/**
 * @brief  The format of the MIPMap subimage is:
 * - 32-bit float
 * - 4 channels (RGBA)
 * - (0, 0) corresponds to the upper-left corner
 * - (1, 0) corresponds to the right of the upper-left corner, etc.
 * - data is stored in row-major order, i.e. elements in the same row are
 *  continuous in memory
 */
class MIPMap {
public:
  enum class ImageWrap { Repeat, Black, Clamp };
  enum class LookUpMethod { EWA, TriLinearInterpolation };
  MIPMap() = default;
  MIPMap(const Vec2u &in_resolution, const vector<Float> &in_data,
      LookUpMethod in_method = LookUpMethod::TriLinearInterpolation,
      ImageWrap in_wrap_mode = ImageWrap::Repeat);
  uint32_t Width() const { return resolution.front()[0]; }
  uint32_t Height() const { return resolution.front()[1]; }
  uint32_t Level() const noexcept { return resolution.size(); }
  Vec3f Texel(uint32_t l, uint32_t s, uint32_t t) const;
  Vec3f LookUp(const Vec2f &st, Float width = 0.f) const noexcept;
  Vec3f LookUp(const Vec2f &st, Vec2f dstdx, Vec2f dstdy) const noexcept;

private:
  Vec3f TriTexel(uint32_t l, const Vec2f &st) const noexcept;
  Vec3f EWA(uint32_t l, const Vec2f &st, const Vec2f &dst0,
      const Vec2f &dst1) const noexcept;

  RDR_FORCEINLINE Vec3f GetOne(
      const Float *data, const Vec2u &res, const Vec2u &st) const noexcept {
    return Vec3f(&data[4 * (res[0] * std::min(st[1], res[1] - 1) +
                               std::min(st[0], res[0] - 1))]);
  }

  Vec3f Interpolate(
      const Float *data, const Vec2u &res, const Vec2f &st) const noexcept {
    const Float s = std::max(res[0] * st[0] - 0.5f, 0.0f);
    const Float t = std::max(res[1] * st[1] - 0.5f, 0.0f);

    const uint32_t lo_s = std::floor(s), hi_s = lo_s + 1;
    const uint32_t lo_t = std::floor(t), hi_t = lo_t + 1;

    return (hi_s - s) * (hi_t - t) * GetOne(data, res, {lo_s, lo_t}) +
           (hi_s - s) * (t - lo_t) * GetOne(data, res, {lo_s, hi_t}) +
           (s - lo_s) * (hi_t - t) * GetOne(data, res, {hi_s, lo_t}) +
           (s - lo_s) * (t - lo_t) * GetOne(data, res, {hi_s, hi_t});
  };

  const LookUpMethod method{LookUpMethod::TriLinearInterpolation};
  const ImageWrap wrap_mode{ImageWrap::Repeat};

  vector<Vec2u> resolution;
  vector<uint32_t> offset;
  vector<Float> data;

  static constexpr Float maxAnisotropy = 8.f;
  static constexpr uint32_t WeightSize = 128;
  static Float gs_weight[WeightSize];
};

RDR_NAMESPACE_END

#endif
