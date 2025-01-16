#include "rdr/integrator.h"

//#include <omp.h>

#include "rdr/bsdf.h"
#include "rdr/camera.h"
#include "rdr/film.h"
#include "rdr/light.h"
#include "rdr/properties.h"
#include "rdr/ray.h"
#include "rdr/scene.h"

RDR_NAMESPACE_BEGIN

void PathIntegrator::render(ref<Camera> camera, ref<Scene> scene) {
  std::atomic<int> cnt    = 0;
  const Vec2i &resolution = camera->getFilm()->getResolution();
  // TODO: fill your implementation here
  // Necessary functions:
  // 1. sampler.setPixelIndex2D (think of how a sampler should be initialized)
  // 2. sampler.getPixelSample (get a pixel sample from the sampler)
  // 3. camera->generateDifferentialRay (don't have to understand)
  // 4. camera->getFilm()->commitSample (commit a sample to the film, remember
  // this understanding)
  // 5. sampler.resetAfterIteration() (not necessary in this sampler
  // 6. Li (don't need to explain, invoke and write its result into the film)
  // implementation, but is indeed necessary in other samplers. If you are
  // interested, go search for Halton and Sobol sampler, they will increase the
  // convergence rate of your renderer by an INCREDIBLE amount almost for free.)
  Sampler sampler;
  for (int x = 0; x < resolution.x; ++x) {
    for (int y = 0; y < resolution.y; ++y) {
      sampler.setPixelIndex2D(Vec2i(x, y));
      for (int s = 0; s < this->spp; ++s) {
        Vec2f sample = sampler.getPixelSample();
        DifferentialRay ray = camera->generateDifferentialRay(sample.x, sample.y);
        Vec3f Li = this->Li(scene, ray, sampler);
        camera->getFilm()->commitSample(sample, Li);
      }
    }
  }
  sampler.resetAfterIteration();
}

/* ===================================================================== *
 *
 * New Integrator's Implementation
 *
 * ===================================================================== */

// Instantiate template
// clang-format off
template Vec3f
IncrementalPathIntegrator::Li<Path>(ref<Scene> scene, DifferentialRay &ray, Sampler &sampler) const;
// clang-format on

// This is exactly a way to separate dec and def
template <typename PathType>
Vec3f IncrementalPathIntegrator::Li(  // NOLINT
    ref<Scene> scene, DifferentialRay &ray, Sampler &sampler) const {
  AssertAllNormalized(ray.direction);
  assert(ray.isValid());
  vector<PathType> paths{};

  Float rr_weight = 1.0;
  Float last_pdf  = 1.0;
  int bounces     = 1;
  bool skip       = false;

  // Result
  Vec3f Li(0.0);  // NOLINT
  PathType base_path(ray, this);

  auto commit_path = [&](const PathType &path) {
    if (deferredEstimate()) {
      paths.push_back(path);
    } else {
      Li += path.estimate();
    }
  };

  /* ===================================================================== *
   * Construct First Interaction
   * =====================================================================
   */
  SurfaceInteraction interaction{};

  bool intersected = scene->intersect(ray, interaction);
  interaction.setPdf(1.0, EMeasure::EUnknownMeasure);

  // Speical judge for light: Le(p1 -> p0)
  if (interaction.isLight() || !intersected) {
    if (intersected) {
      commit_path(base_path.addInteraction(interaction));
    } else {  // no intersection
      // Speical judge for infinite area light
      auto infinite_light = scene->getInfiniteLight();
      if (infinite_light) {
        commit_path(base_path.addInteraction(
            infinite_light->sampleFromOutgoingDirection(-ray.direction)));
      }
    }

    skip = true; /* skip the main loop */
  }

  /* ===================================================================== *
   * Incremental Path Construction
   * =====================================================================
   */

  if (!skip) {
    // Ray hits a non-emitter, compute its ray differentials
    interaction.CalculateRayDifferentials(ray);
  }

  while (bounces < max_depth && !skip) {
    if (!interaction.isValid()) break;

    // refer to lab1_probability_for_rendering.ipynb
    // correctness evaluated
    if (sampler.get1D() < rr_threshold) break;
    rr_weight /= 1.0_F - rr_threshold;

    /* ===================================================================== *
     * Light path construction
     * =====================================================================
     */

    // Sample a light source and set wi is written here. But not the one we want
    auto ref_interaction = interaction;  // reference interaction for light
    auto light_interaction =
        scene->sampleEmitterDirect(ref_interaction, sampler);
    AssertAllNormalized(ref_interaction.wi);
    Ray shadow_ray = light_interaction.isInfLight()
                       ? ref_interaction.spawnRay(ref_interaction.wi)
                       : ref_interaction.spawnRayTo(light_interaction);

    SurfaceInteraction shadow_interaction{};
    bool is_blocked = scene->isBlocked(shadow_ray, shadow_interaction);

    if (!is_blocked && nextEventEstimation()) {
      // Add the light interaction to the path
      auto light_path = base_path;
      light_path.addInteraction(ref_interaction)
          .addInteraction(light_interaction);

      // precision
      if (light_interaction.cosThetaO() <= 0)
        // This exists!
        // should be blocked actually
        goto before_trace;

      if (multipleImportanceSampling()) {
        // Also valid for infinite area light
        const Float light_pdf = Path::toPdfMeasure(
            light_interaction, ref_interaction, EMeasure::ESolidAngle);
        const Float bsdf_pdf = ref_interaction.bsdf->pdf(ref_interaction);
        const Float weight   = miWeight(light_pdf, bsdf_pdf);

        AssertAllNonNegative(light_pdf, bsdf_pdf, weight);
        light_path.setMisWeight(weight);
      }

      light_path.setRrWeight(rr_weight);
      commit_path(light_path);
    }

    /* ===================================================================== *
     * Trace the new ray
     * =====================================================================
     */

before_trace:
    // Create a new direction
    // The interaction is already initialized. Now initialize the PDF
    const auto *bsdf = interaction.bsdf;
    auto bsdf_cache  = bsdf->sample(interaction, sampler, &last_pdf);
    if (SquareNorm(bsdf_cache) < EPS) break;
    AssertAllNormalized(interaction.wi);

    // this is newly generated DifferentialRay
    ray = interaction.spawnRay(interaction.wi);
    if (!ray.isValid()) break;

    // Special case for specular interaction
    if (interaction.isSpecular()) {
      assert(bsdf->isDelta());
      // Set the BSDF cache
      interaction.setBSDFCache(bsdf_cache);
    }

    SurfaceInteraction new_interaction{};

    // Intersect and set interaction.wo
    assert(ray.isValid());
    bool intersected = scene->intersect(ray, new_interaction);

    // Four cases,
    // 1. intersected, normal object
    // 2. intersected, light
    // 3. not intersected, infinite area light
    // 4. not intersected, nothing
    if (intersected) {
      new_interaction.setPdf(last_pdf, EMeasure::ESolidAngle);
    } else if (scene->hasInfiniteLight()) {
      new_interaction = scene->getInfiniteLight()->sampleFromOutgoingDirection(
          -ray.direction);
      new_interaction.setPdf(last_pdf, EMeasure::ESolidAngle);
    } else {
      break;
    }

    // Here, only 1, 2, 3 left
    if (new_interaction.isLight()) {
      // case 2 and 3
      if (randomWalk() || multipleImportanceSampling() ||
          interaction.isSpecular()) {
        auto new_path = base_path;
        new_path.addInteraction(interaction).addInteraction(new_interaction);

        if (multipleImportanceSampling()) {
          const Float bsdf_pdf = last_pdf;
          Float emitter_pdf    = interaction.isSpecular()
                                   ? 0
                                   : scene->pdfEmitterDirect(new_interaction);

          // Convert area light's area PDF to solid angle PDF
          if (!new_interaction.isInfLight()) {
            auto query_interaction = new_interaction;
            query_interaction.setPdf(emitter_pdf, EMeasure::EArea);
            emitter_pdf = Path::toPdfMeasure(
                query_interaction, interaction, EMeasure::ESolidAngle);
          }

          Float weight = miWeight(bsdf_pdf, emitter_pdf);
          new_path.setMisWeight(weight);
        }

        new_path.setRrWeight(rr_weight);
        commit_path(new_path);
      }

      break;
    } else {
      // if is not light, set ray differentials
      new_interaction.CalculateRayDifferentials(ray);
    }

    // Prepare for the next iteration
    // Add the currecnt interaction to the path. UniformSampleOneLight will
    // initialize the wi, so the order should be preserved.
    base_path.addInteraction(interaction);
    interaction = new_interaction;
    ++bounces;
  }

  /* ===================================================================== *
   * Path Summary (if deferred estimate is enabled)
   * =====================================================================
   */
  if (!deferredEstimate()) {
    return Li;
  }

  for (size_t i = 0; i < paths.size(); ++i) {
    const auto &path = paths[i];
    assert(path.verify());
    const auto Pn = path.estimate();  // NOLINT
    AssertAllNonNegative(Pn);

    /**
     * Hook debug point here:
     *   if (sampler.getPixelIndex2D() == Vec2i(x_index, resolution.y -
     * y_index
     * - 1)) {} (x, y) is the index that you would pick from the image viewer,
     * where top-left is (0, 0)
     */

    Li += Pn;
  }

  return Li;
}

RDR_NAMESPACE_END
