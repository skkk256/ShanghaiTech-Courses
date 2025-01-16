#ifndef __SCENE_H__
#define __SCENE_H__

#include <memory>
#include <utility>
#include <vector>

#include "rdr/bvh_tree.h"
#include "rdr/interaction.h"

RDR_NAMESPACE_BEGIN

namespace detail_ {
class BVHPrimitiveNode final : BVHNodeInterface<ref<Primitive>> {
  using DataType = ref<Primitive>;

public:
  BVHPrimitiveNode(DataType data) : data(std::move(data)) {}

  // Implement the interface
  AABB getAABB() const override { return data->getBound(); }
  const DataType &getData() const override { return data; }

private:
  DataType data{nullptr};
};
}  // namespace detail_

/**
 * Scene class capsulates the *structure* of all rendering resources
 * while not owning the resources (@see ref).
 */
class Scene : public ConfigurableObject {
public:
  ~Scene() = default;

  // ++ Required by ConfigurableObject
  Scene(const Properties &props) : ConfigurableObject(props) {}
  void crossConfiguration(const CrossConfigurationContext &context) override;
  // --

  // ++ Required by Object
  void preprocess(const PreprocessContext &context) override;
  // --

  /**
   * @brief Add a initialized primitive to the scene
   */
  void addPrimitive(ref<Primitive> &primitive);

  /**
   * @brief Add a initialized light to the scene
   */
  void addLight(const ref<Light> &light);

  /// Set the infinite light
  void addInfiniteLight(const ref<InfiniteAreaLight> &light);

  /// Have a inifinite light
  bool hasInfiniteLight() const { return infinite_light != nullptr; }

  /// Get the primitives
  const vector<ref<Primitive>> &getPrimitives() const { return primitives; }

  /// Get the lights
  const vector<ref<Light>> &getLights() const { return lights; }

  /// Get the infinite light
  const ref<InfiniteAreaLight> &getInfiniteLight() const {
    return infinite_light;
  }

  bool isBlocked(const Ray &shadow_ray) const;
  bool isBlocked(const Ray &shadow_ray, SurfaceInteraction &interaction) const;

  /// \brief The main ray-primitive intersection routine.
  ///
  /// This accepts a ray and consider its tMin/tMax. If the ray(within this
  /// range) intersects with any primitive, the interaction will be filled.
  /// Otherwise the function will do nothing.
  bool intersect(const Ray &ray, SurfaceInteraction &interaction) const;

  /// Given a surface interaction, return the PDF of sampling this interaction
  /// by light sampling
  Float pdfEmitterDiscrete(const SurfaceInteraction &interaction) const;
  Float pdfEmitterDirect(const SurfaceInteraction &interaction) const;

  /// Sample light sources
  ref<Light> sampleEmitterDiscrete(Sampler &sampler, Float *pmf) const;

  /**
   * @brief Sample a single light-source, fill the reference SurfaceInteraction
   * and return the sampled light interaction.
   *
   * @param interaction The reference SurfaceInteraction
   * @param sampler The sampler
   * @return SurfaceInteraction The sampled light interaction
   */
  SurfaceInteraction sampleEmitterDirect(
      SurfaceInteraction &interaction, Sampler &sampler) const;

  /// Temporary
  AABB getBound() const;

  // ++ Required by Object
  std::string toString() const override {
    return format(
        "Scene [\n"
        "  has_infinite = {}\n"
        "]",
        infinite_light != nullptr);
  }
  // --

private:
  vector<ref<Primitive>> primitives;
  vector<ref<Light>> lights;
  ref<InfiniteAreaLight> infinite_light{nullptr};
  ref<Distribution1D> lights_dist;

  // For interface coherency, let's now use an extra map to maintain the mapping
  // between lights and their Index
  std::map<const Light *, size_t> lights_map;

  /// Scene level accelerator
  BVHTree<detail_::BVHPrimitiveNode> primitive_tree;
};

RDR_REGISTER_CLASS(Scene)

RDR_NAMESPACE_END

#endif
