#include "rdr/render.h"

#include "rdr/all_integrators.h"
#include "rdr/film.h"

RDR_NAMESPACE_BEGIN

void NativeRender::initialize() {
  /* ===================================================================== *
   *
   * Initialize CrossConfigurationContext from Properties
   *
   * ===================================================================== */
  // Initialization is break into steps
  // Pass 1: create all objects with property and fill the
  // CrossConfigurationContext
  cross_context.root_properties = props;
  cross_context.film =
      RDR_CREATE_CLASS(Film, props.getProperty<Properties>("film"));
  cross_context.camera =
      RDR_CREATE_CLASS(Camera, props.getProperty<Properties>("camera"));
  cross_context.scene = RDR_CREATE_CLASS(Scene, props);
  cross_context.integrator =
      RDR_CREATE_CLASS(Integrator, props.getProperty<Properties>("integrator"));

  // Initialize the optional filter
  cross_context.filter = RDR_CREATE_CLASS(ReconstructionFilter,
      props.getProperty<Properties>("film").getProperty<Properties>(
          "filter", Properties{}));  // else return an empty property

  if (props.hasProperty("textures")) {
    auto texture_properties = props.getProperty<Properties>("textures");
    for (const auto &[name, _] : texture_properties)
      cross_context.textures[name] = RDR_CREATE_CLASS(
          Texture, texture_properties.getProperty<Properties>(name));
  }

  if (props.hasProperty("materials")) {
    auto material_properties = props.getProperty<Properties>("materials");
    for (const auto &[name, _] : material_properties)
      cross_context.materials[name] = RDR_CREATE_CLASS(
          BSDF, material_properties.getProperty<Properties>(name));
  }

  if (props.hasProperty("objects")) {
    auto object_properties = props.getProperty<vector<Properties>>("objects");
    for (const auto &primitive_properties : object_properties)
      cross_context.primitives.push_back(
          RDR_CREATE_CLASS(Primitive, primitive_properties));
  }

  if (props.hasProperty("environment_map")) {
    cross_context.environment_map = RDR_CREATE_CLASS(
        InfiniteAreaLight, props.getProperty<Properties>("environment_map"));
  }

  /* ===================================================================== *
   *
   * Perform cross-configuration
   *
   * ===================================================================== */
  // Pass 1.5 initialize global context
  global_context = Factory::Instance().getContext();

  // Pass 2: invoke all crossConfiguration
  for (ConfigurableObject *cobject : global_context)
    cobject->crossConfiguration(cross_context);
}

void NativeRender::clearRuntimeInfo() {
  cross_context      = CrossConfigurationContext{};
  preprocess_context = PreprocessContext{};
  global_context.clear();

  // Must be executed in the last since it releases all the memory allocated
  RenderInterface::clearRuntimeInfo();
}

void NativeRender::preprocess() {
  preprocess_context.scene = cross_context.scene;
  for (ConfigurableObject *cobject : global_context)
    cobject->preprocess(preprocess_context);

    // Debug info
#if false
  for (ConfigurableObject *cobject : global_context)
    Info_("{}", cobject->toString());
#endif
}

void NativeRender::render() {
  // render scene
  cross_context.integrator->render(cross_context.camera, cross_context.scene);
}

bool NativeRender::exportImageToDisk(const fs::path &path) const {
  // save image to disk
  cross_context.film->exportImageToFile(FileResolver::resolveToAbs(path));
  return true;
}

vector<Vec3f> NativeRender::exportImageToArray() const {
  vector<Vec3f> result;
  cross_context.film->exportImageToArray(result);
  return result;
}

Film NativeRender::prepareDebugCanvas(const Vec2i &resolution) {
  Properties props;
  props.setProperty("resolution", resolution);

  // Fake the renderer pipeline
  Film canvas(props);
  CrossConfigurationContext pseudo_context;
  pseudo_context.filter = RDR_CREATE_CLASS(BoxFilter, Properties{});

  canvas.crossConfiguration(pseudo_context);
  canvas.preprocess(PreprocessContext{});

  return canvas;
}

RDR_NAMESPACE_END
