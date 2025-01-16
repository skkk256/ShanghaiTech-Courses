#ifndef __INTERACTION_H__
#define __INTERACTION_H__

#include "rdr/ray.h"
#include "rdr/rdr.h"

RDR_NAMESPACE_BEGIN

/// A list of possible interaction types
/// Only types that can affect integration process are listed here
enum class ESurfaceInteractionType {
  ENone = 0,
  EDiffuse,
  EGlossy,
  ESpecular,
  ELight,
  EInfLight
};

/// If you are not working on any bonus, you can ignore this enum
/// For those who are interested, this enum helps with the implementation of
/// any bidirectional integrators, since the value *bouncing around* is
/// necessarily the We instead of the radiance
enum class ETransportMode { ERadiance = 0, EImportance };

namespace detail_ {
struct InternalSurfaceInteraction {
  ESurfaceInteractionType type{ESurfaceInteractionType::ENone};

  Vec3f p{0.0};
  Vec3f normal{0.0};

  Vec3f wi{0.0};
  Vec3f wo{0.0};

  Float pdf{0.0};
  EMeasure measure{EMeasure::EUnknownMeasure};
  ETransportMode mode{ETransportMode::ERadiance};

  const BSDF *bsdf{nullptr};
  const Light *light{nullptr};
  const Primitive *primitive{nullptr};

  Vec3f bsdf_cache{0.0};

  Vec3f dpdx{}, dpdy{};
  Float dudx{0.0}, dvdx{0.0}, dudy{0.0}, dvdy{0.0};

  Vec2f uv{};
  Vec3f dpdu{}, dpdv{};
  Vec3f dndu{}, dndv{};

  struct Shading {
    Vec3f n{};
    Vec3f dpdu{}, dpdv{};
    Vec3f dndu{}, dndv{};
  } shading{};
};
}  // namespace detail_

/**
 * @brief Interaction along the path.
 * @note Generally its hard to figure out how to fill in this struct
 * step-by-step. This struct is designed with a lot of redundancy. Many fields
 * will not be useful in some certain cases.
 */
struct SurfaceInteraction final {
public:
  // making it easier to read and parse
  template <typename T>
  using WrapperType = std::add_lvalue_reference_t<T>;

  /* ===================================================================== *
   *
   * Constant getters to Internal Data. Designed to avoid any potention error in
   * setup the SurfaceInteraction
   *
   * ===================================================================== */
  SurfaceInteraction()  = default;
  ~SurfaceInteraction() = default;
  SurfaceInteraction(const SurfaceInteraction &other)
      : internal(other.internal) {}
  SurfaceInteraction(SurfaceInteraction &&other) noexcept { swap(other); }
  // Move assignment
  SurfaceInteraction &operator=(SurfaceInteraction other) noexcept {
    swap(other);
    return *this;
  }

  /** === BSDF-related definitions ===*/
  /** the dir of the incoming ray (pointing out with p) in *World Space* */
  WrapperType<Vec3f> wi{internal.wi};
  /** the dir of the outgoing ray (pointing out from p) in *World Space* */
  WrapperType<Vec3f> wo{internal.wo};

  /** === Type definitions === */
  WrapperType<ESurfaceInteractionType> type{internal.type};

  /** === Bidirection === */
  WrapperType<ETransportMode> mode{internal.mode};

  /*************************************/
  // The following variables are const.
  // Can only be modified from setters.
  /************************************/

  /** === General definitions === */
  /** the position of the intersection in *World Space* */
  WrapperType<const Vec3f> p{internal.p};
  /** the normal of the surface at the intersection point */
  WrapperType<const Vec3f> normal{internal.normal};
  /** (u, v) is the parameter coordinate. More specifically,
   ** for example, when the surface interation is on a triangle (p_0, p_1, p_2)
   ** (b_0, b_1, b_2) is Barycentric Coordinates
   ** then we have the equation: p = b_0 * p_0 + b_1 * p_1 + b_2 * p_2
   ** and t_i is the corresponding texture coordinates to p_i
   ** then (u, v) = b_0 * t_0 + b_1 * t_1 + b_2 * t_2
   */
  WrapperType<const Vec2f> uv{internal.uv};
  /** \frac{\partial{p}}{\partial{u}} */
  WrapperType<const Vec3f> dpdu{internal.dpdu};
  /** \frac{\partial{p}}{\partial{v}} */
  WrapperType<const Vec3f> dpdv{internal.dpdv};
  /** \frac{\partial{normal}}{\partial{u}} */
  WrapperType<const Vec3f> dndu{internal.dndu};
  /** \frac{\partial{normal}}{\partial{v}} */
  WrapperType<const Vec3f> dndv{internal.dndv};

  /** === Primitive-related definitions === */
  WrapperType<const BSDF *const> bsdf{internal.bsdf};
  WrapperType<const Light *const> light{internal.light};
  WrapperType<const Primitive *const> primitive{internal.primitive};

  /** === Path-related definitions === */
  /** the PDF of getting this interaction, the sampling PDF of the last
   * operation */
  WrapperType<const Float> pdf{internal.pdf};
  /** the measure of the PDF */
  WrapperType<const EMeasure> measure{internal.measure};

  /** === Specular-related definition === */
  /** the cached BSDF value */
  WrapperType<const Vec3f> bsdf_cache{internal.bsdf_cache};

  /** === Ray differential definitions === */
  /** (x, y) is the screen coordinate */
  /** \frac{\partial{p}}{\partial{x}} */
  WrapperType<const Vec3f> dpdx{internal.dpdx};
  /** \frac{\partial{p}}{\partial{y}} */
  WrapperType<const Vec3f> dpdy{internal.dpdy};
  /** \frac{\partial{u}}{\partial{x}} */
  WrapperType<const Float> dudx{internal.dudx};
  /** \frac{\partial{v}}{\partial{x}} */
  WrapperType<const Float> dvdx{internal.dvdx};
  /** \frac{\partial{u}}{\partial{y}} */
  WrapperType<const Float> dudy{internal.dudy};
  /** \frac{\partial{v}}{\partial{y}} */
  WrapperType<const Float> dvdy{internal.dvdy};

  /** === Shading related definitions === */
  WrapperType<const detail_::InternalSurfaceInteraction::Shading> shading{
      internal.shading};

  /* ===================================================================== *
   *
   * Helper Functions
   *
   * ===================================================================== */

  /// Spawn a ray from the intersection point in the given direction.
  virtual Ray spawnRay(const Vec3f &d) const {
    AssertAllNormalized(d);
    const Vec3f &o = isSpecular() ? p + FaceForward(d, normal) * EPS
                                  : OffsetRayOrigin(p, normal);
    return {o, d, RAY_DEFAULT_MIN, RAY_DEFAULT_MAX};
  }

  /// Spawn a ray from the intersection point to the given point.
  virtual Ray spawnRayTo(const Vec3f &target) const {
    // Should not change the order of calculation to avoid precision issues
    const Vec3f &d   = Normalize(target - p);
    const Vec3f &o   = isSpecular() ? p + FaceForward(d, normal) * EPS
                                    : OffsetRayOrigin(p, normal);
    const Float norm = Norm(target - o);
    return {o, Normalize(target - o), RAY_DEFAULT_MIN, norm * (1.0_F - EPS)};
  }

  /// Spawn a ray from the interaction point to the given interaction.
  virtual Ray spawnRayTo(const SurfaceInteraction &it) const {
    return spawnRayTo(it.p);
  }

  /// Some math utils
  Float cosThetaI() const noexcept { return Dot(shading.n, wi); }
  Float cosThetaO() const noexcept { return Dot(shading.n, wo); }
  Float cosTheta(const Vec3f &w) const noexcept { return Dot(shading.n, w); }

  /// Calculate the ray differentials given interaction.
  void CalculateRayDifferentials(const DifferentialRay &ray);  // NOLINT

  // shortcuts
  bool isRadiance() const noexcept { return mode == ETransportMode::ERadiance; }
  bool isDiffuse() const noexcept {
    return type == ESurfaceInteractionType::EDiffuse;
  }
  bool isGlossy() const noexcept {
    return type == ESurfaceInteractionType::EGlossy;
  }
  bool isSpecular() const noexcept {
    return type == ESurfaceInteractionType::ESpecular;
  }
  bool isLight() const noexcept {
    bool result = type == ESurfaceInteractionType::ELight ||
                  type == ESurfaceInteractionType::EInfLight;
    assert(result ? light != nullptr : true);
    return result;
  }
  bool isInfLight() const noexcept {
    return type == ESurfaceInteractionType::EInfLight;
  }
  bool isGeometry() const noexcept {
    return type == ESurfaceInteractionType::EDiffuse ||
           type == ESurfaceInteractionType::EGlossy ||
           type == ESurfaceInteractionType::ESpecular;
  }

  bool isValid() const {
    switch (type) {
      case ESurfaceInteractionType::ENone:
        Exception_("Interaction type is NONE");

      case ESurfaceInteractionType::ELight:
      case ESurfaceInteractionType::EInfLight:
        return light;  // consider sampleFromDirection
      case ESurfaceInteractionType::EDiffuse:
        return bsdf;
      case ESurfaceInteractionType::EGlossy:
        return bsdf;
      case ESurfaceInteractionType::ESpecular:
        return bsdf;
      default:
        return false;
    }
  }

  std::string toString() const {
    return format(
        "SurfaceInteraction[\n"
        "  p =      {},\n"
        "  normal = {},\n"
        "  wi =     {},\n"
        "  wo =     {},\n"
        "  pdf =    {}\n]",
        p, normal, wi, wo, pdf);
  }

  /* ===================================================================== *
   *
   * Setters to Internal Data. The functions are self-explanatory
   *
   * ===================================================================== */

  void setPrimitive(const BSDF *in_bsdf, const Light *in_light,
      const Primitive *in_primitive) noexcept {
    internal.bsdf      = in_bsdf;
    internal.light     = in_light;
    internal.primitive = in_primitive;
  }

  void setGeneral(const Vec3f &in_p, const Vec3f &in_normal) {
    AssertAllValid(in_p, in_normal);
    AssertAllNormalized(in_normal);
    internal.p      = in_p;
    internal.normal = in_normal;
    // Should set default shading normal
    internal.shading.n = in_normal;
  }

  void setUV(const Vec2f &in_uv) {
    AssertAllValid(uv);
    internal.uv = in_uv;
  }

  void setDifferential(const Vec3f &in_p, const Vec3f &in_normal,
      const Vec2f &uv, const Vec3f &in_dpdu, const Vec3f &in_dpdv,
      const Vec3f &in_dndu, const Vec3f in_dndv) {
    AssertAllValid(in_p, in_normal, uv, in_dpdu, in_dpdv, in_dndu, in_dndv);
    AssertAllNormalized(in_normal);
    AssertNear(abs(in_normal), abs(Normalize(Cross(in_dpdu, in_dpdv))));
    internal.p      = in_p;
    internal.normal = in_normal;
    internal.uv     = uv;
    internal.dpdu   = in_dpdu;
    internal.dpdv   = in_dpdv;
    internal.dndu   = in_dndu;
    internal.dndv   = in_dndv;

    // Should set default shading atrribute
    internal.shading = {in_normal, in_dpdu, in_dpdv, in_dndu, in_dndv};
  }

  void setShading(const Vec3f &in_normal, const Vec3f &in_dpdu,
      const Vec3f &in_dpdv, const Vec3f &in_dndu, const Vec3f in_dndv) {
    AssertAllValid(in_normal, in_dpdu, in_dpdv, in_dndu, in_dndv);
    AssertAllNormalized(in_normal);

    internal.shading = {in_normal, in_dpdu, in_dpdv, in_dndu, in_dndv};
  }

  void setPdf(const Float &in_pdf, const EMeasure &in_measure) {
    AssertAllValid(in_pdf);
    AssertAllNonNegative(in_pdf);
    internal.pdf     = in_pdf;
    internal.measure = in_measure;
  }

  void setBSDFCache(const Vec3f &in_bsdf_cache) {
    AssertAllValid(in_bsdf_cache);
    internal.bsdf_cache = in_bsdf_cache;
  }

  void swap(SurfaceInteraction &other) noexcept {
    std::swap(internal, other.internal);
  }

private:
  // Internal data
  detail_::InternalSurfaceInteraction internal;
};

/// Calculate triangle differentials
void CalculateTriangleDifferentials(SurfaceInteraction &interaction,
    const Vec3f &b, const ref<TriangleMeshResource> &mesh,
    const uint32_t triangle_index);

RDR_NAMESPACE_END

#endif  // __INTERACTION_H__
