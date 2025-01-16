#include "rdr/scene.h"

#include "rdr/accel.h"
#include "rdr/integrator.h"
#include "rdr/light.h"
#include "rdr/primitive.h"

RDR_NAMESPACE_BEGIN

void Scene::crossConfiguration(const CrossConfigurationContext &context) {
  for (const auto &primitive : context.primitives) {
    if (primitive->hasAreaLight()) addLight(primitive->getAreaLight());
    primitives.push_back(primitive);
    primitive_tree.push_back(primitive);
  }

  if (context.environment_map) {
    addInfiniteLight(context.environment_map);
  }

  clearProperties();

  // Still, a hack; make sure that scene bound is built prior to other
  // initialization
  primitive_tree.build();
}

void Scene::preprocess(const PreprocessContext &context) {
  std::vector<Float> weights;
  if (hasInfiniteLight()) {
    for (const auto &_ : lights) weights.push_back(1.0F);
  } else {
    for (const auto &light : lights) {
      weights.push_back(light->energy());
    }
  }

  lights_dist = make_ref<Distribution1D>(weights.data(), weights.size());
}

void Scene::addPrimitive(ref<Primitive> &primitive) {
  primitives.push_back(primitive);
  primitive_tree.push_back(primitive);
}

void Scene::addLight(const ref<Light> &light) {
  lights_map[light.get()] = lights.size();
  lights.push_back(light);
}

void Scene::addInfiniteLight(const ref<InfiniteAreaLight> &light) {
  infinite_light = light;
  addLight(light);
}

bool Scene::isBlocked(const Ray &shadow_ray) const {
  SurfaceInteraction in;
  return isBlocked(shadow_ray, in);
}

bool Scene::isBlocked(
    const Ray &shadow_ray, SurfaceInteraction &interaction) const {
  return intersect(shadow_ray, interaction);
}

bool Scene::intersect(const Ray &ray, SurfaceInteraction &interaction) const {
  Ray new_ray      = ray;
  bool intersected = primitive_tree.intersect(new_ray,
      [&interaction](
          Ray &internal_ray, const ref<Primitive> &primitive) -> bool {
        // If it is requested, it must intersect with this bounding box
        return primitive->intersect(internal_ray, interaction);
      });
  if (intersected) assert(interaction.type != ESurfaceInteractionType::ENone);
  return intersected;
}

Float Scene::pdfEmitterDirect(const SurfaceInteraction &interaction) const {
  assert(interaction.isValid());

  // To eliminate all switch statements with inheritance
  switch (interaction.type) {
    case ESurfaceInteractionType::ELight:
    case ESurfaceInteractionType::EInfLight:
      return interaction.light->pdf(interaction) *
             pdfEmitterDiscrete(interaction);
    default: {
      Exception_("Unsupported interaction type!");
    }
  }
}

Float Scene::pdfEmitterDiscrete(const SurfaceInteraction &interaction) const {
  auto light_iterator = lights_map.find(interaction.light);
  if (light_iterator == lights_map.end()) {
    return 0;
  } else {
    return lights_dist->discretePDF(light_iterator->second);
  }
}

ref<Light> Scene::sampleEmitterDiscrete(Sampler &sampler, Float *pmf) const {
  if (getLights().empty()) Exception_("No light in the scene!");

  auto lights        = getLights();
  const int light_id = lights_dist->sampleDiscrete(sampler.get1D(), pmf);
  AssertAllValid(*pmf);
  assert(0 <= light_id && light_id < lights.size());
  return lights[light_id];
}

SurfaceInteraction Scene::sampleEmitterDirect(
    SurfaceInteraction &interaction, Sampler &sampler) const {
  Float light_pmf        = 0.0;
  auto light             = sampleEmitterDiscrete(sampler, &light_pmf);
  auto light_interaction = light->sample(interaction, sampler);
  light_interaction.setPdf(
      light_interaction.pdf * light_pmf, light_interaction.measure);
  return light_interaction;
}

AABB Scene::getBound() const {
  AABB aabb;
  for (const auto &primitive : primitives)
    aabb = AABB(aabb, primitive->getBound());
  return primitive_tree.getAABB();
  return aabb;
}

RDR_NAMESPACE_END
