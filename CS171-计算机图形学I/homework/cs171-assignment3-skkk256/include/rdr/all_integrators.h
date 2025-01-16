#include "rdr/integrator.h"

RDR_NAMESPACE_BEGIN

RDR_REGISTER_FACTORY(Integrator, [](const Properties &props) -> Integrator * {
  auto type = props.getProperty<std::string>("type", "path");
  if (type == "path") {
    return Memory::alloc<IncrementalPathIntegrator>(props);
  } else if (type == "photon") {
    // possibly your final project?
    UNIMPLEMENTED;
  } else if (type == "sppm") {
    // possibly your final project?
    UNIMPLEMENTED;
  } else if (type == "guided") {
    // possibly your final project?
    UNIMPLEMENTED;
  } else if (type == "bdpt") {
    // possibly your final project?
    UNIMPLEMENTED;
  } else {
    Exception_("Integrator type {} not found", type);
  }

  return nullptr;
})

RDR_NAMESPACE_END
