#ifndef __LIGHT_H__
#define __LIGHT_H__

#include "rdr/interaction.h"
#include "rdr/rdr.h"

RDR_NAMESPACE_BEGIN

/**
 * @brief The general interface of light source.
 */
class Light : public ConfigurableObject {
public:
  virtual ~Light() = default;

  // ++ Required by ConfigurableObject
  Light(const Properties &props) : ConfigurableObject(props) {}
  // --

  /**
   * @brief Query the Le with the interaction **on** (and produced by) the light
   * source and a direction leaving the light source. Because purely
   * Le(position, direction) is not efficient to determined the normal.
   */
  virtual Vec3f Le(
      const SurfaceInteraction &interaction, const Vec3f &w) const = 0;

  /**
   * @brief Given a SurfaceInteraction sampled by the light, compute the PDF of
   * sampling this interaction.
   */
  virtual Float pdf(const SurfaceInteraction &interaction) const = 0;

  /**
   * @brief Given a SurfaceInteraction sampled by the light, compute the PDF of
   * sampling this direction (corresponding to Light::sampleDirection)
   */
  virtual Float pdfDirection(
      const SurfaceInteraction &interaction, const Vec3f &w) const = 0;

  /**
   * @brief Sample a SurfaceInteraction on the light source with a given
   * interaction on *any surface* and write the PDF onto the point.
   */
  virtual SurfaceInteraction sample(
      SurfaceInteraction &interaction, Sampler &sampler) const = 0;

  /**
   * @brief Sample a SurfaceInteraction on the light source without a
   * interaction on any surface.
   */
  virtual SurfaceInteraction sample(Sampler &sampler) const = 0;

  /**
   * @brief Given a SurfaceInteraction on the light source (possibly by
   * Light::sample), sample an outgoing direction based on this interaction.
   * This function is used by bi-directional methods.
   */
  virtual Vec3f sampleDirection(const SurfaceInteraction &interaction,
      Sampler &sampler, Float &pdf) const = 0;

  /**
   * @brief Calculate the energy emitted by this light.
   */
  virtual Float energy() const = 0;
};

/**
 * @brief The interface of area light source. Area lights are interacting with
 * scene in a primitive, and is accessed through SurfaceInteraction.
 */
class AreaLight final : public Light {
public:
  friend class Primitive;

  // ++ Required by ConfigurableObject
  AreaLight(const Properties &props);
  void crossConfiguration(const CrossConfigurationContext &context) override;
  // --

  /// @see Light::Le
  Vec3f Le(
      const SurfaceInteraction &interaction, const Vec3f &w) const override;

  /// @see Light::pdf
  Float pdf(const SurfaceInteraction &interaction) const override;

  /// @see Light::pdfDirection
  Float pdfDirection(
      const SurfaceInteraction &interaction, const Vec3f &w) const override;

  /// @see Light::sample
  SurfaceInteraction sample(
      SurfaceInteraction &interaction, Sampler &sampler) const override;

  /// @see Light::sample
  SurfaceInteraction sample(Sampler &sampler) const override;

  /// @see Light::energy
  Float energy() const override;

  /// @see Light::sampleDirection
  Vec3f sampleDirection(const SurfaceInteraction &interaction, Sampler &sampler,
      Float &pdf) const override;

  // ++ Required by Object
  std::string toString() const override {
    return format(
        "AreaLight[\n"
        "  radiance = {}\n"
        "]",
        radiance);
  }
  // --

protected:
  ref<Shape> shape;
  Vec3f radiance;
};

/**
 * @brief The interface of infinite area light source. Infinite area lights are
 * environment lights, which can useful for debugging in this renderer.
 */
class InfiniteAreaLight final : public Light {
public:
  constexpr static Mat4f TRANSITION_MATRIX = Mat4f{
      {0, 0, -1, 0},
      {1, 0,  0, 0},
      {0, 1,  0, 0},
      {0, 0,  0, 1}
  };

  // ++ Required by ConfigurableObject
  InfiniteAreaLight(const Properties &props)
      : Light(props),
        scale(props.getProperty<Float>("scale", 1.0)),
        transform(props.getProperty<Mat4f>("transform", IdentityMatrix4)) {
    transform         = Mul(transform, TRANSITION_MATRIX);
    transform_inverse = Inverse(transform);
  }

  void crossConfiguration(const CrossConfigurationContext &context) override;
  // --

  /// @see Light::Le. interaction is not used.
  Vec3f Le(
      const SurfaceInteraction &interaction, const Vec3f &w) const override;

  /// @see Light::pdf
  Float pdf(const SurfaceInteraction &interaction) const override;

  /// @see Light::pdfDirection
  Float pdfDirection(
      const SurfaceInteraction &interaction, const Vec3f &w) const override;

  /// @see Light::sample
  SurfaceInteraction sample(
      SurfaceInteraction &interaction, Sampler &sampler) const override;

  /// @see Light::sample
  SurfaceInteraction sample(Sampler &sampler) const override;

  /// @see Light::sampleDirection
  Vec3f sampleDirection(const SurfaceInteraction &interaction, Sampler &sampler,
      Float &pdf) const override;

  /// @see Light::energy
  Float energy() const override { return 0; }

  /// Accept a direction pointing outward and return a SurfaceInteraction.
  SurfaceInteraction sampleFromOutgoingDirection(const Vec3f &w) const;

  // ++ Required by Object
  void preprocess(const PreprocessContext &context) override;

  std::string toString() const override {
    return format(
        "InfiniteAreaLight[\n"
        "  radius = {}\n"
        "]",
        radius);
  }
  // --

  // Temporary interface
  Vec3f getCenter() const { return scene_center; }
  Float getRadius() const { return radius; }
  ref<Texture> &getTexture() { return texture; }

private:
  Vec3f dirWorldToLocal(const Vec3f &w) const {
    return Normalize(Mul(transform_inverse, Vec4f(w, 0)).xyz());
  }

  Vec3f dirLocalToWorld(const Vec3f &w) const {
    return Normalize(Mul(transform, Vec4f(w, 0)).xyz());
  }

  Vec3f pointWorldToLocal(const Vec3f &p) const {
    const auto tmp = Mul(transform_inverse, Vec4f(p, 1));
    return tmp.xyz() / tmp.w;
  }

  Vec3f pointLocalToWorld(const Vec3f &p) const {
    const auto tmp = Mul(transform, Vec4f(p, 1));
    return tmp.xyz() / tmp.w;
  }

  ref<Texture> texture{nullptr};
  // ref<Distribution2D> distribution{nullptr};

  Vec3f scene_center{0.0};
  Float radius{1e3};

  Float scale;
  Mat4f transform, transform_inverse;
};

RDR_REGISTER_CLASS(AreaLight)
RDR_REGISTER_CLASS(InfiniteAreaLight)

RDR_REGISTER_FACTORY(Light, [](const Properties &props) -> Light * {
  auto type = props.getProperty<std::string>("area");
  if (type == "area") {
    return Memory::alloc<AreaLight>(props);
  } else {
    Exception_("Light type {} not supported", type);
  }

  return nullptr;
})

RDR_NAMESPACE_END

#endif
