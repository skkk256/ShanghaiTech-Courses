#ifndef __PATH_H__
#define __PATH_H__

#include <utility>

#include "rdr/interaction.h"

RDR_NAMESPACE_BEGIN

// not strictly CRTP, but a way to enforce the interface while supporting
// *POLYMORPHIC CHAINNING*
// https://en.wikipedia.org/wiki/Method_chaining
// https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern
// you don't have to understand this.
template <typename PathType>
class PathInterface {
public:
  /// Add a new interaction to the path
  virtual PathType &addInteraction(const SurfaceInteraction &interaction) = 0;

  /**
   * @brief Estimate the radiance of the path using the Monte Carlo estimator.
   *
   * Note that is not recommended(but possible) to perform the throughput and
   * pdf calculation separately since their value can be very small with
   * geometric terms.
   *
   * @return Vec3f
   */
  virtual Vec3f estimate() const = 0;

  /// Verify the interactions on the path
  virtual bool verify() const = 0;

  /// Calculate the length of the path
  virtual int length() const = 0;

  /// Set the weight of this path in MIS
  virtual void setMisWeight(Float weight) = 0;

  /// Set the russian-roulette weight of this path
  virtual void setRrWeight(Float weight) = 0;

  /// Print the path
  virtual std::string toString() const { return format("PathInterface[]"); }

  /**
   * Translate between different measures. Given this interaction and the
   * last interaction, calculate the probability of sampling this interaction in
   * the given measure.
   *
   * The derivation is in the document.
   *
   * @param interaction
   * @param last_interaction
   * @param target_measure
   * @return int
   */
  static Float toPdfMeasure(const SurfaceInteraction &interaction,
      const SurfaceInteraction &last_interaction,
      const EMeasure &target_measure) {
    // dw = dA cos(theta) / r^2
    // then cos(theta_A) p(w) / r^2 = p(A)
    if (interaction.measure == target_measure) return interaction.pdf;
    if (interaction.measure == EMeasure::EUnknownMeasure ||
        target_measure == EMeasure::EUnknownMeasure) {
      Exception_("Unknown measure");
    }

    // Fix numerical issue
    const Float square_dist = SquareNorm(interaction.p - last_interaction.p);
    const Float cos_theta   = abs(interaction.cosThetaO());

    // Will be handled later
    if (cos_theta <= 0)
      Error_("non-positive cos_theta encountered (can exist)");
    AssertAllValid(square_dist, cos_theta);
    AssertAllNonNegative(square_dist, cos_theta);

    return interaction.measure == EMeasure::EArea
             ? interaction.pdf * square_dist / cos_theta
             : interaction.pdf * cos_theta / square_dist;
  }

protected:
  Ray ray0;
  const PathIntegrator *integrator;

  // Not a good practice... but I don't have time to change them.
  PathInterface(Ray ray, const PathIntegrator *integrator)
      : ray0(std::move(ray)), integrator(integrator) {}
  ~PathInterface()                                     = default;
  PathInterface(const PathInterface &)                 = default;
  PathInterface(PathInterface &&) noexcept             = default;
  PathInterface &operator=(const PathInterface &other) = default;
};

/**
 * @brief The sampled path created by integrator.
 */
class Path final : PathInterface<Path> {
public:
  using Super = PathInterface<Path>;
  using Super::toPdfMeasure;

  /// Construct with the first ray.
  Path(const Ray &ray, const PathIntegrator *integrator)
      : Super(ray, integrator) {}

  /// @see PathInterface::addInteraction
  Path &addInteraction(const SurfaceInteraction &interaction) override {
    assert(interaction.isValid());
    interactions.push_back(interaction);
    return *this;
  }

  /// @see PathInterface::estimate
  Vec3f estimate() const override;

  /// @see PathInterface::verify
  bool verify() const override;

  /// @see PathInterface::length
  int length() const override { return interactions.size(); }

  /// @see PathInterface::setMisWeight
  void setMisWeight(Float weight) override { mis_weight = weight; }

  /// @see PathInterface::setRrWeight
  void setRrWeight(Float weight) override { rr_weight = weight; }

  /// @see PathInterface::toString
  std::string toString() const override;

private:
  Float mis_weight{1};  //<! The weight of this path in MIS
  Float rr_weight{1};   //<! The weight of this path by rr(correction)
  vector<SurfaceInteraction>
      interactions{};  //<! The interactions along the path, might include light
                       // source
};

RDR_NAMESPACE_END

#endif
