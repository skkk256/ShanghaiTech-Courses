#ifndef __FRESNEL_H__
#define __FRESNEL_H__

#include "rdr/rdr.h"

RDR_NAMESPACE_BEGIN

RDR_FORCEINLINE Float FresnelDielectric(
    Float cos_theta_I, Float etaI, Float etaT) {
  cos_theta_I = std::clamp<Float>(cos_theta_I, -1, 1);

  bool entering = cos_theta_I > 0.f;
  if (!entering) {
    std::swap(etaI, etaT);
    cos_theta_I = abs(cos_theta_I);
  }

  Float sin_theta_I = std::sqrt(std::max(0.0_F, 1 - cos_theta_I * cos_theta_I));
  Float sin_theta_t = etaI / etaT * sin_theta_I;  // snell's law
  if (sin_theta_t >= 1) return 1.;

  Float cos_theta_T = std::sqrt(std::max(0.0_F, 1 - sin_theta_t * sin_theta_t));

  // The formula and code exactly from PBRT
  Float rparl = ((etaT * cos_theta_I) - (etaI * cos_theta_T)) /
                ((etaT * cos_theta_I) + (etaI * cos_theta_T));
  Float rperp = ((etaI * cos_theta_I) - (etaT * cos_theta_T)) /
                ((etaI * cos_theta_I) + (etaT * cos_theta_T));
  return (rparl * rparl + rperp * rperp) / 2;
}

RDR_FORCEINLINE Float FresnelSchlick(
    Float cos_theta_I, Float etaI, Float etaT) {
  // Following the assumption to do some simplification
  cos_theta_I = std::abs(cos_theta_I);
  cos_theta_I = std::max<Float>(0.0, cos_theta_I);
  cos_theta_I = std::min<Float>(1.0, cos_theta_I);
  Float R0    = (etaI - etaT) / (etaI + etaT);
  R0          = R0 * R0;
  return R0 + (1 - R0) * pow(1 - cos_theta_I, 5);
}

RDR_FORCEINLINE Vec3f FresnelConductor(
    Float cos_theta_I, const Vec3f &etaI, const Vec3f &etaT, const Vec3f &k) {
  cos_theta_I = std::clamp<Float>(cos_theta_I, -1, 1);
  Vec3f eta   = etaT / etaI;
  Vec3f eta_k = k / etaI;

  Float cos_theta_I2 = cos_theta_I * cos_theta_I;
  Float sin_theta_I2 = 1 - cos_theta_I2;
  Vec3f eta2         = eta * eta;
  Vec3f eta_k2       = eta_k * eta_k;

  Vec3f t0         = eta2 - eta_k2 - Vec3f(sin_theta_I2);
  Vec3f a2_plus_b2 = (t0 * t0 + 4 * eta2 * eta_k2);
  a2_plus_b2[0]    = std::sqrt(a2_plus_b2[0]);
  a2_plus_b2[1]    = std::sqrt(a2_plus_b2[1]);
  a2_plus_b2[2]    = std::sqrt(a2_plus_b2[2]);
  Vec3f t1         = a2_plus_b2 + Vec3f(cos_theta_I2);
  Vec3f a          = 0.5_F * (a2_plus_b2 + t0);
  a[0]             = std::sqrt(a[0]);
  a[1]             = std::sqrt(a[1]);
  a[2]             = std::sqrt(a[2]);
  Vec3f t2         = (Float)2 * cos_theta_I * a;
  Vec3f rs         = (t1 - t2) / (t1 + t2);

  Vec3f t3 = cos_theta_I2 * a2_plus_b2 + Vec3f(sin_theta_I2 * sin_theta_I2);
  Vec3f t4 = t2 * sin_theta_I2;
  Vec3f rp = rs * (t3 - t4) / (t3 + t4);

  return (rp + rs) * 0.5_F;
}

RDR_NAMESPACE_END

#endif
