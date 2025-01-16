#include "rdr/path.h"

#include "rdr/bsdf.h"
#include "rdr/light.h"
#include "rdr/scene.h"

RDR_NAMESPACE_BEGIN

/* ===================================================================== *
 * It is your turn!
 * =====================================================================
 */

Vec3f Path::estimate() const {
  // !NOTE: you should always multiply your result with mis_weight and
  // !rr_weight.
  // !HINT: break here with a debugger might be helpful(that's my approach)? Or
  // !you can print out the interactions one by one.
  Vec3f L{0.0};
  Vec3f throughout{1.0};
  switch (this->length()) {
  case 0:  // nothing
    // TODO: fill your implementation here
    break;
  case 1: {
    // TODO: fill your implementation here
    // Possible functions:
    // 1. interaction.isLight
    // 2. light->Le
    //UNIMPLEMENTED;
    SurfaceInteraction interaction = interactions.back();
    if (interaction.isLight()) {
      L = interaction.light->Le(interaction, interaction.wo);
    }
    break;
  }  // Calculate Le(p1 -> p0)
  default: {
    assert(interactions.back().isLight());

    /* ===================================================================== *
     * Throughput Calculation
     * =====================================================================
     */

    // TODO: fill your implementation here
    // Possible members or functions that you might use:
    // 1. interaction.bsdf
    // 2. interaction.wi/wo
    // 3. interaction.shading.n
    // 4. bsdf->evaluate()
    // 5. toPdfMeasure()
    // 6. a for loop
    // UNIMPLEMENTED;
    for (size_t i = 0; i < interactions.size() - 2; ++i) {
      SurfaceInteraction last_interaction = interactions[i];
      SurfaceInteraction interaction = interactions[i + 1];
      Float cosThetaI = last_interaction.cosThetaI();
//      Float cosTheta2 = Dot(next_interaction.shading.n, wi);
//      Float G = cosTheta1 * cosTheta2 / Dot(interaction.p - next_interaction.p, interaction.p - next_interaction.p);
      Float pdf = toPdfMeasure(interaction, last_interaction,EMeasure::ESolidAngle);
      throughout *= last_interaction.bsdf->evaluate(last_interaction) * abs(cosThetaI) / pdf;
    }
    /* ===================================================================== *
     * Light-contribution Calculation
     * =====================================================================
     */

    // TODO: fill your implementation here
    // Possible usage:
    // 1. interaction.bsdf
    // 2. interaction.wi/wo
    // 3. interaction.shading.n
    // 4. bsdf->evaluate()
    // 5. toPdfMeasure() (with EMeasure::ESolidAngle or EMeasure::EArea)
    // 6. interaction.cosThetaI()
    // and more...
    size_t n = interactions.size() - 2;

    SurfaceInteraction last_interaction = interactions[n];
    SurfaceInteraction interaction = interactions[n + 1];
    Float cosThetaI = last_interaction.cosThetaI();
    Float cosThetaO = interaction.cosThetaO();
    Float G = abs(cosThetaI) * abs(cosThetaO) / Dot(interaction.p - last_interaction.p, interaction.p - last_interaction.p);
    Float pdf = toPdfMeasure(interaction, last_interaction,EMeasure::EArea);
    throughout *= last_interaction.bsdf->evaluate(last_interaction) * G / pdf;

//    const auto &interaction = interactions.back();
    L = interaction.light->Le(interaction, interaction.wo);
    L *= throughout;
    break;
   }
  }
  return L * mis_weight * rr_weight;
}

bool Path::verify() const {
  bool result = true;
  if (this->length() == 0) return result;

  result &= interactions[0].wo == -ray0.direction;
  AssertAllNormalized(interactions[0].wo);
  AssertAllNormalized(ray0.direction);
  for (size_t i = 0; i < interactions.size() - 1; ++i) {
    auto interaction = interactions[i];
    auto primitive   = interaction.primitive;
    auto bsdf        = interaction.bsdf;

    result &= primitive != nullptr;
    result &= bsdf != nullptr;
    result &= AllClose(interaction.wi, -interactions[i + 1].wo);
    AssertAllNormalized(interaction.wi);
  }

  return result;
}

std::string Path::toString() const {
  // https://graphics.stanford.edu/courses/cs348b-01/course29.hanrahan.pdf
  std::ostringstream ss;
  ss << "Path["
     << "E" << ToString(ray0.origin);

  if (!interactions.empty()) ss << " -> ";
  for (size_t i = 0; i < interactions.size(); ++i) {
    switch (interactions[i].type) {
      case ESurfaceInteractionType::EDiffuse:
        ss << "D";
        break;
      case ESurfaceInteractionType::EGlossy:
        ss << "G";
        break;
      case ESurfaceInteractionType::ESpecular:
        ss << "S";
        break;
      case ESurfaceInteractionType::EInfLight:
        ss << "IL";
        break;
      default:
        ss << "N";
        break;
    }

    ss << "[p" << ToString(interactions[i].p) << ", ";
    ss << "n" << ToString(interactions[i].normal) << ", ";
    ss << "wi" << ToString(interactions[i].wi) << "]";
    if (i < interactions.size() - 1) ss << " -> ";
  }

  ss << "]";
  return ss.str();
}

RDR_NAMESPACE_END
