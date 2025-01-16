/**
 * @file rfilter.h
 * @author ShanghaiTech CS171 TAs
 * @brief Screen reconstruction filter
 * @version 0.1
 * @date 2023-07-28
 *
 * @copyright Copyright (c) 2023
 */

#ifndef __RFILTER_H__
#define __RFILTER_H__

#include "rdr/factory.h"
#include "rdr/object.h"

RDR_NAMESPACE_BEGIN

/// Abstract class for screen reconstruction filter
/// Borrowed partially from Mitsuba 0.6 and pbrt
// radius is defined on a square [-radius, radius] x [-radius, radius] centered
// at (pixel + 0.5).
class ReconstructionFilter : public ConfigurableObject {
public:
  virtual ~ReconstructionFilter() = default;

  /// Evaluate the filter function
  virtual Float evaluate(const Vec2f &relative_pos) const = 0;
  RDR_FORCEINLINE Float getRadius() const { return radius; }

protected:
  // ++ Required by ConfigurableObject
  ReconstructionFilter(const Properties &props)
      // by default, the radius should be 0.5
      : ConfigurableObject(props),
        radius(props.getProperty<Float>("radius", 0.5)) {}
  // --

  RDR_FORCEINLINE bool isInside(const Vec2f &relative_pos) const {
    return (std::abs(relative_pos.x) <= radius) &&
           (std::abs(relative_pos.y) <= radius);
  }

  Float radius;
};

// =======================================================================
// See
// https://pbr-book.org/3ed-2018/Sampling_and_Reconstruction/Image_Reconstruction
// =======================================================================

class BoxFilter final : public ReconstructionFilter {
public:
  // ++ Required by ConfigurableObject
  BoxFilter(const Properties &props) : ReconstructionFilter(props) {}
  // --

  Float evaluate(const Vec2f &relative_pos) const override {
    return isInside(relative_pos) ? 1.0 : 0.0;
  }
};

class GaussianFilter final : public ReconstructionFilter {
public:
  // ++ Required by ConfigurableObject
  GaussianFilter(const Properties &props)
      : ReconstructionFilter(props),
        alpha(props.getProperty<Float>("alpha", 2.0)),
        exp_x(std::exp(-alpha * radius * radius)),
        exp_y(std::exp(-alpha * radius * radius)) {}
  // --

  Float evaluate(const Vec2f &relative_pos) const override {
    return isInside(relative_pos)
             ? gaussian(relative_pos.x, exp_x) * gaussian(relative_pos.y, exp_y)
             : 0;
  }

private:
  Float alpha, exp_x, exp_y;
  RDR_FORCEINLINE Float gaussian(Float d, Float expv) const {
    return Max(static_cast<Float>(0),
        static_cast<Float>(std::exp(-alpha * d * d) - expv));
  }
};

class MitchellFilter final : public ReconstructionFilter {
public:
  // ++ Required by ConfigurableObject
  MitchellFilter(const Properties &props)
      : ReconstructionFilter(props),
        B(props.getProperty<Float>("B", 1.0 / 3.0)),
        C(props.getProperty<Float>("C", 1.0 / 3.0)) {}
  // --

  Float evaluate(const Vec2f &relative_pos) const override {
    return isInside(relative_pos) ? mitchell1D(relative_pos.x / radius) *
                                        mitchell1D(relative_pos.y / radius)
                                  : 0;
  }

private:
  Float B, C;
  // ref:
  // https://pbr-book.org/3ed-2018/Sampling_and_Reconstruction/Image_Reconstruction#x1-MitchellFilter
  RDR_FORCEINLINE Float mitchell1D(Float x) const {
    x = std::abs(2 * x);
    if (x > 1)
      return ((-B - 6 * C) * x * x * x + (6 * B + 30 * C) * x * x +
                 (-12 * B - 48 * C) * x + (8 * B + 24 * C)) *
             (1.f / 6.f);
    else
      return ((12 - 9 * B - 6 * C) * x * x * x +
                 (-18 + 12 * B + 6 * C) * x * x + (6 - 2 * B)) *
             (1.f / 6.f);
  }
};

RDR_REGISTER_CLASS(BoxFilter)
RDR_REGISTER_CLASS(GaussianFilter)
RDR_REGISTER_CLASS(MitchellFilter)
RDR_REGISTER_FACTORY(ReconstructionFilter,
    [](const Properties &props) -> ReconstructionFilter * {
      auto type = props.getProperty<std::string>("type", "box");
      if (type == "box") {
        return Memory::alloc<BoxFilter>(props);
      } else if (type == "gaussian") {
        return Memory::alloc<GaussianFilter>(props);
      } else if (type == "mitchell") {
        return Memory::alloc<MitchellFilter>(props);
      } else {
        Exception_("Filter type {} not found", type);
      }

      return nullptr;
    })

RDR_NAMESPACE_END

#endif
