/**
 * @file integrator.h
 * @author ShanghaiTech CS171 TAs
 * @brief The CORE part of any renderer. Perform Monte Carlo integration on path
 * space. Our integrator is designed for teaching purpose.
 * @version 0.1
 * @date 2023-04-13
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef __INTEGRATOR_H__
#define __INTEGRATOR_H__

#include "rdr/interaction.h"
#include "rdr/path.h"

RDR_NAMESPACE_BEGIN

class Integrator : public ConfigurableObject {
public:
  Integrator(const Properties &props) : ConfigurableObject(props) {}

  virtual void render(ref<Camera> camera, ref<Scene> scene) = 0;
  std::string toString() const override                     = 0;
};

/// Retained for debugging
class PathIntegrator : public Integrator {
public:
  PathIntegrator(const Properties &props)
      : Integrator(props),
        max_depth(props.getProperty<int>("max_depth", 12)),
        spp(props.getProperty<int>("spp", 32)) {}

  /// @see Integrator::render
  void render(ref<Camera> camera, ref<Scene> scene) override;

  /**
   * @brief The core function of path tracing. Perform Monte Carlo integration
   * given a ray, estimate the radiance as definition.
   */
  virtual Vec3f Li(  // NOLINT
      ref<Scene> scene, DifferentialRay &ray, Sampler &sampler) const = 0;

  std::string toString() const override {
    std::ostringstream ss;
    ss << "PathIntegrator[\n"
       << format("  max_depth = {}\n", max_depth) << format("  spp = {}\n", spp)
       << "]";
    return ss.str();
  }

protected:
  int max_depth, spp;
};

/**
 * @brief This is a simple and inefficient path tracer, which is different from
 * what you can see online. The calculation of radiance is splited into two
 * different phases, path construction and Monte Carlo integration on path. You
 * should refer to the notes for formulas.
 */
class IncrementalPathIntegrator final : public PathIntegrator {
public:
  /**
   * @brief The profile of the integrator for you to do experiments.
   * - ERandomWalk: Random walk on path space
   * - ENextEventEstimation: Perform light sampling
   * - EMultipleImportanceSampling: Perform MIS
   */
  enum class IntegratorProfile {
    ERandomWalk                 = 0,
    ENextEventEstimation        = 1,
    EMultipleImportanceSampling = 2,
  };

  // Another setting for Integrator to promote *performance* for debugging
  enum class EstimatorProfile {
    EImmediateEstimate = 0,  // for performance
    EDeferredEstimate  = 1,  // for debug
  };

  IncrementalPathIntegrator(const Properties &props)
      : PathIntegrator(props),
        rr_threshold(props.getProperty<Float>("rr_threshold", 0.1)) {
    // might be necessary to understand? just a json object.
    auto profile_name = props.getProperty<std::string>("profile", "NEE");
    if (profile_name == "RW" || profile_name == "RandomWalk") {
      profile = IntegratorProfile::ERandomWalk;
    } else if (profile_name == "NEE" || profile_name == "NextEventEstimation") {
      profile = IntegratorProfile::ENextEventEstimation;
    } else if (profile_name == "MIS" ||
               profile_name == "MultipleImportanceSampling") {
      profile = IntegratorProfile::EMultipleImportanceSampling;
    } else {
      Exception_("Profile name {} not supported; use MIS", profile_name);
      profile = IntegratorProfile::EMultipleImportanceSampling;
    }
  }

  /// @see Integrator::Li
  template <typename PathType>
  Vec3f Li(ref<Scene> scene, DifferentialRay &ray, Sampler &sampler) const;

  /// @see Integrator::Li
  Vec3f Li(
      ref<Scene> scene, DifferentialRay &ray, Sampler &sampler) const override {
    return Li<Path>(scene, ray, sampler);
  }

  // ++ Required by Object
  std::string toString() const override {
    return format(
        "IncrementalPathIntegrator[\n"
        "  max_depth              = {}\n"
        "  spp                    = {}\n"
        "  rr_threshold           = {}\n"
        "  (randomWalk, NEE, MIS) = ({}, {}, {})\n",
        "  (immediate, deferred)  = ({}, {})\n"
        "]",
        max_depth, spp, rr_threshold, randomWalk(), nextEventEstimation(),
        multipleImportanceSampling(), !deferredEstimate(), deferredEstimate());
  }
  // --

protected:
  Float rr_threshold{0.1};

  /// The profile of the integrator
  IntegratorProfile profile{IntegratorProfile::ENextEventEstimation};
  EstimatorProfile eprofile{EstimatorProfile::EImmediateEstimate};

  /// Perform random walk
  RDR_FORCEINLINE bool randomWalk() const {
    return profile == IntegratorProfile::ERandomWalk;
  }

  /// Perform NEE
  RDR_FORCEINLINE bool nextEventEstimation() const {
    return profile >= IntegratorProfile::ENextEventEstimation;
  }

  /// Perform MIS
  RDR_FORCEINLINE bool multipleImportanceSampling() const {
    return profile >= IntegratorProfile::EMultipleImportanceSampling;
  }

  /// Use Deferred Estimate
  RDR_FORCEINLINE bool deferredEstimate() const {
    return eprofile >= EstimatorProfile::EDeferredEstimate;
  }

  /// Heuristic function for MIS
  RDR_FORCEINLINE Float miWeight(Float pdfA, Float pdfB) const {  // NOLINT
    pdfA *= pdfA;
    pdfB *= pdfB;
    return pdfA / (pdfA + pdfB);
  }
};

// CObject Registration
RDR_REGISTER_CLASS(IncrementalPathIntegrator)

RDR_NAMESPACE_END

#endif
