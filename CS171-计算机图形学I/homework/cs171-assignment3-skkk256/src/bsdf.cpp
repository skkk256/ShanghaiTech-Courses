#include "rdr/bsdf.h"

#include "rdr/fresnel.h"
#include "rdr/interaction.h"

RDR_NAMESPACE_BEGIN

namespace {
Vec3f obtainOrientedNormal(
    const SurfaceInteraction &interaction, bool twosided) {
  AssertAllValid(interaction.shading.n);
  AssertAllNormalized(interaction.shading.n);
  return twosided && interaction.cosThetaO() < 0 ? -interaction.shading.n
                                                 : interaction.shading.n;
}
}  // namespace

/* ===================================================================== *
 *
 * IdealDiffusion
 *
 * ===================================================================== */

void IdealDiffusion::crossConfiguration(
    const CrossConfigurationContext &context) {
  auto texture_name = properties.getProperty<std::string>("texture_name");
  auto texture_ptr  = context.textures.find(texture_name);
  if (texture_ptr != context.textures.end()) {
    texture = texture_ptr->second;
  } else {
    Exception_("Texture [ {} ] not found", texture_name);
  }

  clearProperties();
}

Vec3f IdealDiffusion::evaluate(SurfaceInteraction &interaction) const {
  const Vec3f normal = obtainOrientedNormal(interaction, twosided);
  if (Dot(interaction.wi, normal) < 0 || Dot(interaction.wo, normal) < 0)
    return {0, 0, 0};
  return texture->evaluate(interaction) * INV_PI;
}

Float IdealDiffusion::pdf(SurfaceInteraction &interaction) const {
  const Vec3f normal = obtainOrientedNormal(interaction, twosided);
  Float cos_theta    = Dot(interaction.wi, normal);
  return std::max<Float>(cos_theta, EPS) * INV_PI;  // one-sided
}

Vec3f IdealDiffusion::sample(
    SurfaceInteraction &interaction, Sampler &sampler, Float *out_pdf) const {
  const auto local_wi = CosineSampleHemisphere(sampler.get2D());
  const Vec3f normal  = obtainOrientedNormal(interaction, twosided);

  Frame frame(normal);
  interaction.wi = Normalize(frame.LocalToWorld(local_wi));
  if (out_pdf != nullptr) *out_pdf = this->pdf(interaction);
  return this->evaluate(interaction);
}

bool IdealDiffusion::isDelta() const {
  return false;
}

/* ===================================================================== *
 *
 * FresnelSpecular
 *
 * ===================================================================== */
Float Clamp(Float value, Float min, Float max) {
  if (value < min)
    return min;
  else if (value > max)
    return max;
  else
    return value;
}

//Float FresnelDielectric(Float cosThetaI, Float eta) {
//  return Vec3f(1.);
////    cosThetaI = Clamp(cosThetaI, -1, 1);
////    if (cosThetaI < 0) {
////      eta = 1 / eta;
////      cosThetaI = -cosThetaI;
////    }
////    Float sin2Theta_i = 1 - sqrt(cosThetaI);
////    Float sin2Theta_t = sin2Theta_i / sqrt(eta);
////    if (sin2Theta_t >= 1)
////      return 1.f;
////    Float cosTheta_t = sqrt(1 - sin2Theta_t);
////    Float r_parl = (eta * cosThetaI - cosTheta_t) /
////                   (eta * cosThetaI + cosTheta_t);
////    Float r_perp = (cosThetaI - eta * cosTheta_t) /
////                   (cosThetaI + eta * cosTheta_t);
////    return (sqrt(r_parl) + sqrt(r_perp)) / 2;
//}

Glass::Glass(const Properties &props)
    : R(props.getProperty<Vec3f>("R", Vec3f(1.0))),
      T(props.getProperty<Vec3f>("T", Vec3f(1.0))),
      eta(props.getProperty<Float>("eta", 1.5F)),
      BSDF(props) {}

Vec3f Glass::evaluate(SurfaceInteraction &interaction) const {
  // Check if the incoming and outgoing directions are opposite
  if (Dot(interaction.wi, interaction.wo) > 0) {
    // Compute the Fresnel reflectance
    Float Fr = FresnelDielectric(CosTheta(interaction.wi), eta, eta);

    // Return the BSDF value
    return Fr * R / AbsCosTheta(interaction.wo);
  } else {
    return {0, 0, 0};
  }
}

Float Glass::pdf(SurfaceInteraction &interaction) const {
  // Check if the incoming and outgoing directions are opposite
  if (Dot(interaction.wi, interaction.wo) > 0) {
    return 1;
  } else {
    return 0;
  }
}

Vec3f Glass::sample(
    SurfaceInteraction &interaction, Sampler &sampler, Float *pdf) const {
  // Compute the Fresnel reflectance
  Float Fr = FresnelDielectric(interaction.cosThetaI(), 1.0, eta);

  // Check if the ray is reflected or refracted
  if (sampler.get1D() < Fr) {
    // Reflection
    interaction.wi = Reflect(interaction.wo, interaction.normal);
    *pdf = Fr;
  } else {
    // Refraction
    Refract(interaction.wo, interaction.normal, eta, interaction.wi);
    *pdf = 1 - Fr;
  }

  return this->evaluate(interaction);
}

bool Glass::isDelta() const {
  return true;
}

/* ===================================================================== *
 *
 * MicrofacetReflection
 *
 * ===================================================================== */
class BeckmannDistribution {
public:
  static Float D(const Vec3f &wh, const Vec3f &normal, Float alpha) {
    Float tan2Theta = Tan2Theta(wh);
    if (std::isinf(tan2Theta)) return 0;
    Float cos4Theta = Cos2Theta(wh) * Cos2Theta(wh);
//    Float e = std::exp(-tan2Theta * (Cos2Phi(wh) / (alphax * alphax) +
//                                     Sin2Phi(wh) / (alphay * alphay)));
    Float e = std::exp(-tan2Theta  / (alpha * alpha));
    Float b = PI * alpha * alpha * cos4Theta;
    return e/b;
  }

  static Float Lambda(const Vec3f &w, Float alpha) {
//    Float absTanTheta = std::abs(TanTheta(w));
//    if (std::isinf(absTanTheta)) return 0.;
//    // Compute _alpha_ for direction _w_
//    Float alpha =
//        std::sqrt(Cos2Phi(w) * alpha_x * alpha_x + Sin2Phi(w) * alpha_y * alpha_y);
//    Float a = 1 / (alpha * absTanTheta);
//    if (a >= 1.6f) return 0;
//    return (1 - 1.259f * a + 0.396f * a * a) / (3.535f * a + 2.181f * a * a);
    Float tan2theta = Tan2Theta(w);
    if (std::isinf(tan2theta)) return 0;
    Float a = std::sqrt(1 + 1 / (alpha * alpha * tan2theta));
    return (a - 1) / 2;
  }


  static Float G(const Vec3f &wi, const Vec3f &wo, Float alpha) {
//    return std::min(1.0f, std::min(2 * Dot(normal, wh) * Dot(normal, wo) / Dot(wo, wh), 2 * Dot(normal, wh) * Dot(normal, wi) / Dot(wo, wh)));
    return 1 / (1 + Lambda(wo, alpha) + Lambda(wi, alpha));
  }

//  static Float pdf(const Vec3f &wh, const Vec3f &normal, Float alpha_x, Float alpha_y) {
//    Float pdf = D(wh, normal, alpha_x, alpha_y) * AbsCosTheta(wh);
//    return pdf;
//  }

  static Vec3f sample(const Vec3f &normal, Float alpha, Sampler &sampler) {
    Float tan2Theta = alpha * alpha * (-std::log(1 - sampler.get1D()));
    Float phi = 2 * PI * sampler.get1D();
    Float cosTheta = 1 / std::sqrt(1 + tan2Theta);
    Float sinTheta = std::sqrt(std::max(0.0f, 1 - cosTheta * cosTheta));
    Vec3f wh = Normalize(SphericalDirection(sinTheta, cosTheta, phi));
    if (!SameHemisphere(normal, wh)) wh = -wh;
    return wh;
  }
};


void MicrofacetReflection::crossConfiguration(
    const CrossConfigurationContext &context) {
  // TODO(bonus): your implementation here
  auto texture_name = properties.getProperty<std::string>("texture_name");
  auto texture_ptr  = context.textures.find(texture_name);
  if (texture_ptr != context.textures.end()) {
    texture = texture_ptr->second;
  } else {
    Exception_("Texture [ {} ] not found", texture_name);
  }
  clearProperties();
}

MicrofacetReflection::MicrofacetReflection(const Properties &props)
    : etaI(props.getProperty<Vec3f>("etaI", Vec3f(1.0))),
      etaT(props.getProperty<Vec3f>("etaT", Vec3f(1.0))),
      k(props.getProperty<Vec3f>("k", Vec3f(1.0))),
      alpha(props.getProperty<Float>("alpha", 0.0)),
      BSDF(props) {}
  // TODO(bonus): your implementation here

Vec3f MicrofacetReflection::evaluate(SurfaceInteraction &interaction) const {
  // Compute the half-vector
  Vec3f normal = obtainOrientedNormal(interaction, false);
  Vec3f wh = interaction.wo + interaction.wi;

  if (AbsCosTheta(interaction.wi) == 0 || AbsCosTheta(interaction.wo) == 0) return Vec3f(0.0);

  wh = Normalize(wh);
  // Compute the Fresnel term
//  Vec3f F = FresnelConductor(Dot(interaction.wi, wh), etaI, etaT, k);
  Vec3f F = FresnelConductor(interaction.cosThetaI(), etaI, etaT, k);

  // Compute the distribution term
  Float D = BeckmannDistribution::D(wh, normal, alpha);

  // Compute the geometric term
  Float G = BeckmannDistribution::G(interaction.wi, interaction.wo, alpha);
//  if (D == 0.0 || G == 0.0 || F == Vec3f(0.0)) {
//    Info_("D = {}, G = {}, F = {}", D, G, F);
//  }

  // Compute the BSDF value
  return texture->evaluate(interaction) * F * D * G / (4 * AbsCosTheta(interaction.wi) * AbsCosTheta(interaction.wo));
}

Float MicrofacetReflection::pdf(SurfaceInteraction &interaction) const {
  // Compute the half-vector
  const Vec3f normal  = obtainOrientedNormal(interaction, false);
  Vec3f wh = Normalize(interaction.wo + interaction.wi);
  Float D = BeckmannDistribution::D(wh, normal, alpha);
  Float G = BeckmannDistribution::G(interaction.wi, interaction.wo, alpha);
  return D*G*max(0.0f, Dot(interaction.wi, wh)) / Dot(interaction.wo, normal);
  // Compute the PDF
//  return BeckmannDistribution::pdf(wh, interaction.normal, alpha_x, alpha_y) / (4 * Dot(interaction.wo, wh));
}

Vec3f MicrofacetReflection::sample(SurfaceInteraction &interaction, Sampler &sampler, Float *pdf) const {
  // Sample a direction from the microfacet distribution
  const auto local_wi = CosineSampleHemisphere(sampler.get2D());
  const Vec3f normal  = obtainOrientedNormal(interaction, false);
  const Vec3f bnormal = BeckmannDistribution::sample(normal, alpha, sampler);

  Frame frame(bnormal);
  interaction.wi = Normalize(frame.LocalToWorld(local_wi));
  // Compute the PDF
  if (pdf != nullptr) *pdf = this->pdf(interaction);

  // Return the sampled direction
  return this->evaluate(interaction);
}
  bool MicrofacetReflection::isDelta() const {
  // TODO(bonus): your implementation here
  return false;
}

RDR_NAMESPACE_END