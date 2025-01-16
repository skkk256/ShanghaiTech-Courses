#include "rdr/shape.h"

#include <math.h>

#include "linalg.h"
#include "rdr/accel.h"
#include "rdr/bvh_accel.h"
#include "rdr/canary.h"
#include "rdr/interaction.h"
#include "rdr/load_obj.h"
#include "rdr/ray.h"

RDR_NAMESPACE_BEGIN

Sphere::Sphere(const Properties &props)
    : Shape(props),
      center(props.getProperty<Vec3f>("center", Vec3f(0, 0, 0))),
      radius(props.getProperty<Float>("radius", 1)) {}

bool Sphere::intersect(Ray &ray, SurfaceInteraction &interaction) const {
  using InternalScalarType = Double;
  using InternalVecType    = Vec<InternalScalarType, 3>;
  const InternalVecType &o = Cast<InternalScalarType>(ray.origin);
  const InternalVecType &d = Normalize(Cast<InternalScalarType>(ray.direction));
  const InternalVecType &p = Cast<InternalScalarType>(center);

  InternalScalarType t, t1, t2;
  {  // quadratic
    /* Ray intersect with sphere
    ** ||o + td - p||_2^2 = r^2
    ** ||o - p||_2^2 + 2 * dot(o - p, d) * t + ||d||_2^2 * t^2 = r^2
    ** ||d||_2^2 * t^2 + 2 * dot(o - p, d) * t + ||o - p||_2^2 - r^2 = 0
    ** a = ||d||_2^2 = 1, b = 2 * dot(o - p, d), c = ||o - p||_2^2 - r^2
    */
    const InternalVecType &omp  = o - p;
    const InternalScalarType &a = 1, b = 2 * Dot(d, omp),
                             c = SquareNorm(omp) - radius * radius;

    if (b * b <= 4 * a * c) return false;
    const InternalScalarType &delta = sqrt(b * b - 4 * a * c);

    /* Numerically stable quadratic equation solver at^2 + bt + c = 0
    ** Reference: https://people.csail.mit.edu/bkph/articles/Quadratics.pdf
    */
    if (b >= 0) {
      const InternalScalarType &tmp = -b - delta;
      t1 = tmp / (2 * a), t2 = 2 * c / tmp;
    } else {
      const InternalScalarType &tmp = delta - b;
      t1 = 2 * c / tmp, t2 = tmp / (2 * a);
    }
  }

  assert(t1 <= t2);
  if (ray.withinTimeRange(t1) || ray.withinTimeRange(t2)) {
    t = ray.withinTimeRange(t1) ? t1 : t2;
  } else {
    return false;
  }

  InternalVecType position = o + t * d;

  InternalVecType delta_p = position - p;

  // Refine sphere intersection point
  delta_p *= static_cast<InternalScalarType>(radius) / Norm(delta_p);
  if (delta_p.x == 0 && delta_p.y == 0)
    delta_p.x = 1e-8 * static_cast<InternalScalarType>(radius);

  position = delta_p + p;

  constexpr InternalScalarType phiMax   = 2 * PI;
  constexpr InternalScalarType thetaMax = PI;

  InternalScalarType phi = std::atan2(delta_p.y, delta_p.x);
  if (phi < 0) phi += 2 * PI;

  InternalScalarType theta = std::acos(delta_p.z / radius);

  InternalScalarType u = phi / phiMax, v = theta / thetaMax;

  InternalScalarType z_radius =
      std::sqrt(delta_p.x * delta_p.x + delta_p.y * delta_p.y);
  InternalScalarType inv_z_radius = 1 / z_radius;

  InternalScalarType cos_phi = delta_p.x * inv_z_radius;
  InternalScalarType sin_phi = delta_p.y * inv_z_radius;

  /** === Differentiation process === */
  /** p = (x, y, z) + center
   **
   ** x = r * sin(theta) * cos(phi)
   ** y = r * sin(theta) * sin(phi)
   ** z = r * cos(theta)
   **
   ** u = phi / (2 * pi) in [0, 1]
   ** v = theta / pi     in [0, 1]
   **
   ** dpdu = (dxdu, dydu, dzdu)
   **      = (dxdphi, dydphi, dzdphi) * dphidu
   **      = (-y, x, 0) * 2 * pi
   **
   ** dpdv = (dxdv, dydv, dzdv)
   **      = (dxdtheta, dydtheta, dzdtheta) * dthetadv
   **      = (z * cos(phi), z * sin(phi), -r * sin(theta))
   **      = (z * cos(phi), z * sin(phi), -sqrt(r * r - z * z))
   **
   ** reference: 1. https://thesis.library.caltech.edu/1713/12/11_appendixC.pdf
   **            2. https://mathworld.wolfram.com/WeingartenEquations.html
   **            3. (Textbooks in Mathematics) Elsa Abbena, Simon Salamon,
   **               Alfred Gray - Modern Differential Geometry of Curves and
   **               Surfaces with Mathematica-Chapman and Hall_CRC (2006)
   **               P394 Theorem 13.16.
   **
   ** n = dpdu x dpdv / |dpdu x dpdv|
   **
   ** dndu = -S(dpdu) =   (fF - eG) / (EG - F^2) * dpdu
   **                   + (eF - fE) / (EG - F^2) * dpdv
   **
   ** dndv = -S(dpdv) =   (gF - fG) / (EG - F^2) * dpdu
   **                   + (fF - gE) / (EG - F^2) * dpdv
   */

  InternalVecType dpdu = phiMax * InternalVecType(-delta_p.y, delta_p.x, 0);
  InternalVecType dpdv =
      thetaMax * InternalVecType(delta_p.z * cos_phi, delta_p.z * sin_phi,
                     -radius * std::sin(theta));

  InternalVecType d2pduu =
      -phiMax * phiMax * InternalVecType(delta_p.x, delta_p.y, 0);
  InternalVecType d2pduv =
      phiMax * thetaMax * delta_p.z * InternalVecType(-sin_phi, cos_phi, 0);
  InternalVecType d2pdvv = -thetaMax * thetaMax * delta_p;

  InternalScalarType E = Dot(dpdu, dpdu);
  InternalScalarType F = Dot(dpdu, dpdv);
  InternalScalarType G = Dot(dpdv, dpdv);

  InternalVecType N = Normalize(Cross(dpdu, dpdv));

  InternalScalarType e = Dot(N, d2pduu);
  InternalScalarType f = Dot(N, d2pduv);
  InternalScalarType g = Dot(N, d2pdvv);

  InternalScalarType invEGF2 = 1 / (E * G - F * F);

  InternalVecType dndu =
      ((f * F - e * G) * dpdu + (e * F - f * E) * dpdv) * invEGF2;
  InternalVecType dndv =
      ((g * F - f * G) * dpdu + (f * F - g * E) * dpdv) * invEGF2;

  interaction.setDifferential(Cast<Float>(position),
      Cast<Float>(Normalize(delta_p)),
      {static_cast<Float>(v), static_cast<Float>(u)}, Cast<Float>(dpdv),
      Cast<Float>(dpdu), Cast<Float>(dndv), Cast<Float>(dndu));

  ray.setTimeMax(t);
  return true;
}

Float Sphere::area() const {
  return 4 * PI * radius * radius;
}

SurfaceInteraction Sphere::sample(Sampler &sampler) const {
  Vec3f w = UniformSampleSphere(sampler.get2D());
  Vec3f p = center + radius * w;

  SurfaceInteraction interaction{};
  interaction.setGeneral(p, w);
  interaction.setPdf(1.0 / area(), EMeasure::EArea);

  return interaction;
}

AABB Sphere::getBound() const {
  return AABB(center - radius - EPS, center + radius + EPS);
}

Float Sphere::pdf(const SurfaceInteraction &interaction) const {
  return 1.0 / area();
}

TriangleMesh::TriangleMesh(const Properties &props)
    : Shape(props), mesh(make_ref<TriangleMeshResource>()) {
  auto path = props.getProperty<std::string>("path");
  path      = FileResolver::resolveToAbs(path);

  const auto transform = props.getProperty<Mat4f>("transform", IdentityMatrix4);
  const auto translate = props.getProperty<Vec3f>("translate", Vec3f(0.0));
  const auto normal_transform = Transpose(Inverse(transform));

  LoadObj(path, mesh->vertices, mesh->normals, mesh->texture_coordinates,
      mesh->v_indices, mesh->n_indices, mesh->t_indices);

  if (mesh->vertices.empty())
    Exception_("Empty mesh is not allowed from [ {} ]", path);
  std::transform(mesh->vertices.cbegin(), mesh->vertices.cend(),
      mesh->vertices.begin(), [&](const auto &vertex) {
        const auto tmp = Mul(transform, Vec4f(vertex, 1.0));
        return Vec3f(tmp.xyz()) / tmp.w + translate;
      });
  std::transform(mesh->normals.cbegin(), mesh->normals.cend(),
      mesh->normals.begin(), [&](const auto &normal) {
        return Mul(normal_transform, Vec4f(normal, 0)).xyz();
      });
  mesh->has_normal  = !mesh->normals.empty();
  mesh->has_texture = !mesh->texture_coordinates.empty();

#ifdef USE_EMBREE
  accel = make_ref<ExternalBVHAccel>();
#else
  accel = make_ref<BVHAccel>();
#endif

  accel->setTriangleMesh(mesh.get());
  accel->build();

  // Calculate the area of each triangle.
  int n_triangles = mesh->v_indices.size() / 3;
  for (int i = 0; i < n_triangles; ++i) {
    Vec3f v0 = mesh->getVertex(i * 3);
    Vec3f v1 = mesh->getVertex(i * 3 + 1);
    Vec3f v2 = mesh->getVertex(i * 3 + 2);
    areas.push_back(0.5f * Norm(Cross(v1 - v0, v2 - v0)));
    AssertAllPositive(areas.back());
    total_area += areas.back();
  }

  // Reorder vertices to ensure correct normal interpolation
  if (mesh->has_normal) {
    // When the shading normal and calculated face normal is not matched, flip
    // the face normal by reordering the vertices

    auto obtain_face_normal = [&](const std::size_t triangle_id) {
      const Vec3f v0 = mesh->getVertex(triangle_id * 3);
      const Vec3f v1 = mesh->getVertex(triangle_id * 3 + 1);
      const Vec3f v2 = mesh->getVertex(triangle_id * 3 + 2);
      return Normalize(Cross(v1 - v0, v2 - v0));
    };

    // Normals might be inconsistent
    for (std::size_t i = 0; i < n_triangles; ++i) {
      if (Dot(obtain_face_normal(i), mesh->normals[mesh->n_indices[i * 3]]) <
          0) {
        std::swap(mesh->v_indices[i * 3 + 1], mesh->v_indices[i * 3 + 2]);
        std::swap(mesh->n_indices[i * 3 + 1], mesh->n_indices[i * 3 + 2]);
      }
    }
  }

  // Initialize the distribution.
  dist = make_ref<Distribution1D>(areas.data(), n_triangles);
}

bool TriangleMesh::intersect(Ray &ray, SurfaceInteraction &interaction) const {
  bool intersect = accel->intersect(ray, interaction);
  return intersect;
}

Float TriangleMesh::area() const {
  return total_area;
}

SurfaceInteraction TriangleMesh::sample(Sampler &sampler) const {
  Float dist_pdf        = NAN;
  size_t triangle_index = dist->sampleDiscrete(sampler.get1D(), &dist_pdf);
  assert(triangle_index < areas.size());

  Vec3f v0 = mesh->getVertex(triangle_index * 3);
  Vec3f v1 = mesh->getVertex(triangle_index * 3 + 1);
  Vec3f v2 = mesh->getVertex(triangle_index * 3 + 2);

  // Sample a point on the triangle.
  // ha, https://pharr.org/matt/blog/2019/02/27/triangle-sampling-1
  const Vec3f &barycentric = UniformSampleTriangle(sampler.get2D());

  // Interpolate in barycentric coordinates.
  SurfaceInteraction interaction;
  interaction.setGeneral(
      barycentric.x * v0 + barycentric.y * v1 + barycentric.z * v2,
      Normalize(Cross(v1 - v0, v2 - v0)));
  interaction.setPdf(dist_pdf / areas[triangle_index], EMeasure::EArea);

  return interaction;
}

AABB TriangleMesh::getBound() const {
  return accel->getBound();
}

Float TriangleMesh::pdf(const SurfaceInteraction &) const {
  return 1.0_F / total_area;
}

RDR_NAMESPACE_END
