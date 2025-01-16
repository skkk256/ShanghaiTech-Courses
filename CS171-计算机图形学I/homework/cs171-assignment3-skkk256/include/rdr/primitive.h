#ifndef __PRIMITIVE_H__
#define __PRIMITIVE_H__

#include "rdr/rdr.h"

RDR_NAMESPACE_BEGIN

/**
 * @brief Primitives are the aggregate of geometry(class Shape) and
 * material(class BSDF). They are the first-class citizens in the scene.
 */
class Primitive final : public ConfigurableObject {
public:
  Primitive(const Primitive &)            = default;
  Primitive(Primitive &&)                 = delete;
  Primitive &operator=(const Primitive &) = delete;
  Primitive &operator=(Primitive &&)      = delete;

  // ++ Required by ConfigurableObject
  Primitive(const Properties &props);
  void crossConfiguration(const CrossConfigurationContext &context) override;
  // --

  /// Different initialization scehemes
  ~Primitive() override = default;

  /**
   * @brief Intersect a ray with the primitive. Invoke the underlying shape's
   * intersect, and fill the interaction.material if possible.
   *
   * Notice that in this codebase, if Ray is passed with reference but not
   * const-reference, its ray.tMax will possibly be modified. And in most
   * functions, tMin/tMax will be considered.
   *
   * @param ray
   * @param interaction
   * @return bool representing whether the ray intersects with the primitive
   */
  virtual bool intersect(Ray &ray, SurfaceInteraction &interaction) const;

  /// Return the bounding box of the primitive
  virtual AABB getBound() const;

  /// Return the material of the primitive if exists
  virtual ref<BSDF> getMaterial() const noexcept { return bsdf; }

  /// Return the area light of the primitive.
  virtual ref<AreaLight> getAreaLight() const noexcept { return area_light; }

  bool hasMaterial() const noexcept { return bsdf != nullptr; }
  bool hasAreaLight() const noexcept { return area_light != nullptr; }

private:
  ref<Shape> shape{nullptr};
  ref<BSDF> bsdf{nullptr};
  ref<AreaLight> area_light{nullptr};
};

RDR_REGISTER_CLASS(Primitive)

RDR_NAMESPACE_END

#endif
