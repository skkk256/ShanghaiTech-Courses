#ifndef __BSDF_H__
#define __BSDF_H__

#include "rdr/rdr.h"
#include "rdr/texture.h"

RDR_NAMESPACE_BEGIN

/**
 * @brief The base class of BSDF, i.e. material abstraction
 */
class BSDF : public ConfigurableObject {
public:
  enum class EBSDFType {
    // Basic types
    BSDF_DIFFUSE  = 1,
    BSDF_SPECULAR = 2,
    BSDF_GLOSSY   = 3,
    BSDF_COUNT
  };

  virtual ~BSDF() = default;

  // ++ Required by ConfigurableObject
  BSDF(const Properties &props) : ConfigurableObject(props) {}
  // --

  /**
   * @brief Evaluate the BSDF for a given surface interaction.
   *
   * @param interaction The surface interaction at the point of interest.
   * @return The BSDF value.
   */
  virtual Vec3f evaluate(SurfaceInteraction &interaction) const = 0;

  /**
   * @brief Given the interaction sampled by a BSDF, evaluate the probability of
   * sampling this interaction.
   *
   * @param interaction The surface interaction to be filled
   * @return Float the PDF of obtaining this sample
   */
  virtual Float pdf(SurfaceInteraction &interaction) const = 0;

  /**
   * @brief Actually perform a sample with *proper* information provided in
   * interaction
   *
   * @param interaction The surface interaction to be filled
   * @param sampler The sampler to be used
   * @return Float the PDF of obtaining this sample
   */
  virtual Vec3f sample(
      SurfaceInteraction &interaction, Sampler &sampler, Float *pdf) const = 0;

  /**
   * @brief Return if the BSDF is a delta function, i.e., delta(omega_i -
   * omega_o). Used by specular reflection or transmission.
   */
  virtual bool isDelta() const = 0;

protected:
};

class IdealDiffusion final : public BSDF {
public:
  // ++ Required by ConfigurableObject
  IdealDiffusion(const Properties &props)
      : BSDF(props), twosided(props.getProperty<bool>("twosided", false)) {}
  void crossConfiguration(const CrossConfigurationContext &context) override;
  std::string toString() const override {
    std::ostringstream ss;
    ss << "IdealDiffusion[\n"
       << format("  texture = {}\n", texture->toString())
       << format("  twosided = {}\n", twosided) << "]";
    return ss.str();
  }
  // --

  /// @see BSDF::evaluate
  Vec3f evaluate(SurfaceInteraction &interaction) const override;

  /// @see BSDF::pdf
  Float pdf(SurfaceInteraction &interaction) const override;

  /// @see BSDF::sample
  Vec3f sample(SurfaceInteraction &interaction, Sampler &sampler,
      Float *pdf = nullptr) const override;

  /// @see BSDF::isDelta
  bool isDelta() const override;

private:
  ref<Texture> texture;

  // Alway orient the normal to the same hemisphere of wo
  bool twosided{false};
};

class Glass final : public BSDF {
public:
  // ++ Required by ConfigurableObject
  Glass(const Properties &props);
  std::string toString() const override {
    return format(
        "Glass[\n"
        "  R   = {}\n"
        "  T   = {}\n"
        "  eta = {}\n"
        "]",
        R, T, eta);
  }
  // --

  /// @see BSDF::evaluate
  Vec3f evaluate(SurfaceInteraction &interaction) const override;

  /// @see BSDF::pdf
  Float pdf(SurfaceInteraction &interaction) const override;

  /// @see BSDF::sample
  Vec3f sample(SurfaceInteraction &interaction, Sampler &sampler,
      Float *pdf = nullptr) const override;

  /// @see BSDF::isDelta
  bool isDelta() const override;

private:
  const Vec3f R, T;
  const Float eta;
};

class MicrofacetReflection final : public BSDF {
public:
  // ++ Required by ConfigurableObject
  MicrofacetReflection(const Properties &props);
  void crossConfiguration(const CrossConfigurationContext &context) override;
  // --

  /// @see BSDF::evaluate
  Vec3f evaluate(SurfaceInteraction &interaction) const override;

  /// @see BSDF::pdf
  Float pdf(SurfaceInteraction &interaction) const override;

  /// @see BSDF::sample
  Vec3f sample(SurfaceInteraction &interaction, Sampler &sampler,
      Float *pdf = nullptr) const override;

  /// @see BSDF::isDelta
  bool isDelta() const override;

private:
  ref<Texture> texture;
  Float alpha;
  Vec3f etaI, etaT, k;
  // TODO(bonus): your member variables here
};

RDR_REGISTER_CLASS(IdealDiffusion)
RDR_REGISTER_CLASS(Glass)
RDR_REGISTER_CLASS(MicrofacetReflection)
RDR_REGISTER_FACTORY(BSDF, [](const Properties &props) -> BSDF * {
  auto type = props.getProperty<std::string>("type", "diffuse");
  if (type == "diffuse") {
    return Memory::alloc<IdealDiffusion>(props);
  } else if (type == "glass") {
    // TODO(bonus): your implementation here
    return Memory::alloc<Glass>(props);
//    UNIMPLEMENTED;
  } else if (type == "roughconductor") {
    // TODO(bonus): your implementation here
    return Memory::alloc<MicrofacetReflection>(props);;
  } else {
    Exception_("Material type {} not supported", type);
  }

  return nullptr;
})

RDR_NAMESPACE_END

#endif