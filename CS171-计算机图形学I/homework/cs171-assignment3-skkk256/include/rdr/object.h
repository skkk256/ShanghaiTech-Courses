#ifndef __OBJECT_H__
#define __OBJECT_H__

#include "rdr/fwd.h"
#include "rdr/math_aliases.h"
#include "rdr/platform.h"
#include "rdr/properties.h"
#include "rdr/std.h"

RDR_NAMESPACE_BEGIN

// used by the second pass of initialization
struct CrossConfigurationContext {
  using MaterialMapType   = std::map<std::string, ref<BSDF>>;
  using TextureMapType    = std::map<std::string, ref<Texture>>;
  using PrimitiveListType = vector<ref<Primitive>>;

  ref<Film> film;
  ref<ReconstructionFilter> filter;
  ref<Camera> camera;
  ref<Scene> scene;
  ref<Integrator> integrator;

  MaterialMapType materials;
  TextureMapType textures;
  PrimitiveListType primitives;

  ref<InfiniteAreaLight> environment_map;

  Properties root_properties;
};

struct PreprocessContext {
  ref<Scene> scene;
};

/**
 * The absolute base object of all scene objects
 */
class Object {
public:
  Object()          = default;
  virtual ~Object() = default;

  virtual void preprocess(const PreprocessContext &context) {}
  virtual std::string toString() const = 0;
};

/**
 * @class ConfigurableObject
 * @brief An object which supports construction from a Properties instance.
 */
class ConfigurableObject : public Object {
public:
  // virtual void ConstructFromProperties(const Properties &props) noexcept = 0;

  /// crossConfiguration will be invoked right after the initialization of all
  /// classes
  virtual void crossConfiguration(const CrossConfigurationContext &context) {
    // do nothing here
  }

  std::string toString() const override {
    return format("ConfigurableObject [ {} ]", properties.toString());
  }

protected:
  /// Construct an empty Propertis
  ConfigurableObject() = default;

  /// Construct from properties
  ConfigurableObject(const Properties &props) : properties(props) {}

  /// Construct from rvalue properties
  ConfigurableObject(Properties &&props) noexcept
      : properties(std::move(props)) {}

  /// Copy constructor
  ConfigurableObject(const ConfigurableObject &cobject)
      : properties(cobject.properties) {}

  /// Move constructor
  ConfigurableObject(ConfigurableObject &&cobject) noexcept
      : properties(std::move(cobject.properties)) {}

  ~ConfigurableObject() noexcept override = default;

  /// Clear properties for clear debugging
  void clearProperties() noexcept { properties.clear(); }

  Properties properties;
};

RDR_NAMESPACE_END

#endif