#include "rdr/primitive.h"

#include "rdr/accel.h"
#include "rdr/bsdf.h"
#include "rdr/interaction.h"
#include "rdr/light.h"
#include "rdr/shape.h"

RDR_NAMESPACE_BEGIN

Primitive::Primitive(const Properties &props) : ConfigurableObject(props) {
  // A very complex initiaization
  shape = RDR_CREATE_CLASS(Shape, props);
  if (props.hasProperty("light")) {
    area_light =
        RDR_CREATE_CLASS(AreaLight, props.getProperty<Properties>("light"));
    area_light->shape = shape;
  }
}

void Primitive::crossConfiguration(const CrossConfigurationContext &context) {
  if (properties.hasProperty("material_name")) {
    auto material_name = properties.getProperty<std::string>("material_name");
    auto material_ptr  = context.materials.find(material_name);
    if (material_ptr != context.materials.end()) bsdf = material_ptr->second;
  } else if (!properties.hasProperty("light")) {
    Exception_("Primitive is initialized without both material and light");
  }

  // Clear the properties for debugging propose
  clearProperties();
}

bool Primitive::intersect(Ray &ray, SurfaceInteraction &interaction) const {
  if (shape->intersect(ray, interaction)) {
    if (bsdf) {
      // primitive is responsible for setting these
      if (bsdf->isDelta()) {
        interaction.type = ESurfaceInteractionType::ESpecular;
      } else if (dynamic_cast<MicrofacetReflection *>(bsdf.get()) != nullptr) {
        interaction.type = ESurfaceInteractionType::EGlossy;
      } else {
        interaction.type = ESurfaceInteractionType::EDiffuse;
      }
    }

    // not for bi-direction method
    interaction.wo = -ray.direction;

    // set the type of interaction
    // which is set to GEOMETRY if light is not presented
    if (area_light) interaction.type = ESurfaceInteractionType::ELight;
    interaction.setPrimitive(bsdf.get(), area_light.get(), this);

    return true;
  }

  return false;
}

AABB Primitive::getBound() const {
  return shape->getBound();
}

RDR_NAMESPACE_END
