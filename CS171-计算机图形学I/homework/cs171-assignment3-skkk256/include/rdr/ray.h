#ifndef __RAY_H__
#define __RAY_H__

#include <utility>

#include "rdr/canary.h"
#include "rdr/math_aliases.h"
#include "rdr/rdr.h"

RDR_NAMESPACE_BEGIN

namespace detail_ {
struct InternalRay {
  Vec3f origin{};
  Vec3f direction{};
  Vec3f safe_inverse_direction{};
  Float t_min{RAY_DEFAULT_MIN};
  Float t_max{RAY_DEFAULT_MAX};
};

struct InternalDifferentialRay {
  bool has_differential{false};
  Vec3f dx_origin{}, dy_origin{};
  Vec3f dx_direction{}, dy_direction{};
};
}  // namespace detail_

struct Ray {
  template <typename T>
  using WrapperType = std::add_lvalue_reference_t<T>;

  /// origin point of ray
  WrapperType<const Vec3f> origin{internal.origin};
  /// normalized direction of the ray
  WrapperType<const Vec3f> direction{internal.direction};

  /// min and max distance of the ray
  WrapperType<const Float> t_min{internal.t_min};
  WrapperType<const Float> t_max{internal.t_max};

  /// Used by AABB intersection
  WrapperType<const Vec3f> safe_inverse_direction{
      internal.safe_inverse_direction};

  Ray() = default;
  Ray(const Vec3f &o, const Vec3f &dir, Float t_min = RAY_DEFAULT_MIN,
      Float t_max = RAY_DEFAULT_MAX)
      : internal{o, dir, getSafeInverseDirection(dir), t_min, t_max} {
    AssertAllValid(o, dir, safe_inverse_direction, t_min, t_max);
    AssertAllNormalized(dir);
  }

  Ray(const Ray &other) : internal(other.internal) {}
  Ray(Ray &&other) noexcept : internal(other.internal) {}

  Vec3f operator()(Float t) const noexcept { return origin + t * direction; }

  Ray &operator=(Ray other) noexcept {
    swap(other);
    return *this;
  }

  // helper
  bool withinTimeRange(const Float &t) const {
    return t_min <= t && t <= t_max;
  }

  bool isValid() const noexcept {
    return t_min <= t_max && IsAllNormalized(direction) &&
           IsAllValid(t_min, t_max, direction, origin);
  }

  void setGeneral(const Vec3f &o, const Vec3f &d) {
    AssertAllValid(o, d);
    AssertAllNormalized(d);
    internal.origin    = o;
    internal.direction = d;
  }

  void setTimeRange(const Float &in_t_min, const Float &in_t_max) {
    AssertAllValid(in_t_min, in_t_max);
    internal.t_min = in_t_min;
    internal.t_max = in_t_max;
  }

  void setTimeMax(const Float &in_t_max) {
    AssertAllValid(in_t_max);
    internal.t_max = in_t_max;
  }

  void swap(Ray &other) noexcept { std::swap(internal, other.internal); }

private:
  detail_::InternalRay internal;

  static Vec3f getSafeInverseDirection(const Vec3f &local_direction) {
    Vec3f result;
    for (int i = 0; i < 3; ++i)
      result[i] = abs(local_direction[i]) < EPS
                    ? 1 / (Sign(local_direction[i]) * EPS)
                    : 1 / local_direction[i];
    return result;
  }
};

struct DifferentialRay final : public Ray {
  template <typename T>
  using WrapperType = std::add_lvalue_reference_t<T>;

  /// whether the this ray has differential
  WrapperType<const bool> has_differential{
      differential_internal.has_differential};
  /// origin point of ray in (x + dx, y)
  WrapperType<const Vec3f> dx_origin{differential_internal.dx_origin};
  /// normalized direction of the ray in (x + dx, y)
  WrapperType<const Vec3f> dx_direction{differential_internal.dx_direction};
  /// origin point of ray in (x, y + dy)
  WrapperType<const Vec3f> dy_origin{differential_internal.dy_origin};
  /// normalized direction of the ray in (x, y + dy)
  WrapperType<const Vec3f> dy_direction{differential_internal.dy_direction};

  DifferentialRay() = default;

  DifferentialRay(const Vec3f &o, const Vec3f &d, const Vec3f &dx_o,
      const Vec3f &dx_d, const Vec3f &dy_o, const Vec3f &dy_d,
      const bool &in_has_differential = false,
      const Float &in_t_min           = RAY_DEFAULT_MIN,
      const Float &in_t_max           = RAY_DEFAULT_MAX)
      : Ray(o, d, in_t_min, in_t_max),
        differential_internal{in_has_differential, dx_o, dy_o, dx_d, dy_d} {
    AssertAllValid(dx_o, dx_d, dy_o, dy_d);
    AssertAllNormalized(dx_d, dy_d);
  }

  DifferentialRay(Ray &&ray, const Ray &dx_ray, const Ray &dy_ray)
      : Ray(ray),
        differential_internal{true, dx_ray.origin, dy_ray.origin,
            dx_ray.direction, dy_ray.direction} {
    AssertAllNormalized(direction, dx_direction, dy_direction);
    AssertAllValid(
        direction, dx_direction, dy_direction, origin, dx_origin, dy_origin);
  }

  DifferentialRay(const Vec3f &o, const Vec3f &dir,
      Float t_min = RAY_DEFAULT_MIN, Float t_max = RAY_DEFAULT_MAX)
      : Ray(o, dir, t_min, t_max) {
    differential_internal.has_differential = false;
  }

  DifferentialRay(const DifferentialRay &other)
      : Ray(other), differential_internal(other.differential_internal) {}

  DifferentialRay(DifferentialRay &&other) noexcept
      : Ray(std::move(other)),
        differential_internal(other.differential_internal) {}

  DifferentialRay &operator=(DifferentialRay other) noexcept {
    swap(other);
    return *this;
  }

  DifferentialRay &operator=(Ray other) noexcept {
    Ray::operator=(std::move(other));
    differential_internal.has_differential = false;
    return *this;
  }

  void swap(DifferentialRay &other) noexcept {
    Ray::swap(other);
    std::swap(differential_internal, other.differential_internal);
  }

  bool isValid() const noexcept {
    return Ray::isValid() &&
           IsAllValid(dx_origin, dx_direction, dy_origin, dy_direction);
  }

  void setDifferential(const Vec3f &dx_o, const Vec3f &dx_d, const Vec3f &dy_o,
      const Vec3f &dy_d) {
    AssertAllValid(dx_o, dx_d, dy_o, dy_d);
    AssertAllNormalized(dx_d, dy_d);
    differential_internal.has_differential = true;
  }

private:
  detail_::InternalDifferentialRay differential_internal;
};

RDR_NAMESPACE_END

#endif
