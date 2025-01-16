#include "rdr/accel.h"

#include "rdr/canary.h"
#include "rdr/interaction.h"
#include "rdr/math_aliases.h"
#include "rdr/platform.h"
#include "rdr/shape.h"

RDR_NAMESPACE_BEGIN

/* ===================================================================== *
 *
 * AABB Implementations
 *
 * ===================================================================== */

bool AABB::isOverlap(const AABB &other) const {
  return ((other.low_bnd[0] >= this->low_bnd[0] &&
              other.low_bnd[0] <= this->upper_bnd[0]) ||
             (this->low_bnd[0] >= other.low_bnd[0] &&
                 this->low_bnd[0] <= other.upper_bnd[0])) &&
         ((other.low_bnd[1] >= this->low_bnd[1] &&
              other.low_bnd[1] <= this->upper_bnd[1]) ||
             (this->low_bnd[1] >= other.low_bnd[1] &&
                 this->low_bnd[1] <= other.upper_bnd[1])) &&
         ((other.low_bnd[2] >= this->low_bnd[2] &&
              other.low_bnd[2] <= this->upper_bnd[2]) ||
             (this->low_bnd[2] >= other.low_bnd[2] &&
                 this->low_bnd[2] <= other.upper_bnd[2]));
}

bool AABB::intersect(const Ray &ray, Float *t_in, Float *t_out) const {
  // ray distance for two intersection points are returned by pointers.
  const Vec3f &inverse_ray_direction = ray.safe_inverse_direction;

  const Vec3f &t1 = (low_bnd - ray.origin) * inverse_ray_direction;
  const Vec3f &t2 = (upper_bnd - ray.origin) * inverse_ray_direction;

  const Vec3f &t_min = Min(t1, t2);
  const Vec3f &t_max = Max(t1, t2);
  *t_in              = ReduceMax(t_min);
  *t_out             = ReduceMin(t_max);

  /* When tOut < 0 and the ray is intersecting with AABB, the whole AABB is
   * behind us */
  *t_in  = max(*t_in, ray.t_min);
  *t_out = min(*t_out, ray.t_max);
  if (*t_out < 0) return false;
  return *t_out >= *t_in;
}

/* ===================================================================== *
 *
 * Accelerator Implementations
 *
 * ===================================================================== */

bool TriangleIntersect(Ray &ray, const uint32_t &triangle_index,
    const ref<TriangleMeshResource> &mesh, SurfaceInteraction &interaction) {
  using InternalScalarType = Double;
  using InternalVecType    = Vec<InternalScalarType, 3>;

  AssertAllValid(ray.direction, ray.origin);
  AssertAllNormalized(ray.direction);

  const auto &vertices = mesh->vertices;
  const Vec3u v_idx(&mesh->v_indices[3 * triangle_index]);
  assert(v_idx.x < mesh->vertices.size());
  assert(v_idx.y < mesh->vertices.size());
  assert(v_idx.z < mesh->vertices.size());

  InternalVecType dir = Cast<InternalScalarType>(ray.direction);
  InternalVecType v0  = Cast<InternalScalarType>(vertices[v_idx[0]]);
  InternalVecType v1  = Cast<InternalScalarType>(vertices[v_idx[1]]);
  InternalVecType v2  = Cast<InternalScalarType>(vertices[v_idx[2]]);

  // TODO: fill in your implementation here.
  // Some of the code is already provided for you. You are free to discard them.
  // Calculate these scalars from the above information.
  // You can replace `InternalScalarType`, etc. with, for example, float or
  // double.
  InternalScalarType u, v, t /* fill them with correct values */;
  // Calculate the determinant of the matrix
  InternalVecType O = Cast<InternalScalarType>(ray.origin);
  InternalVecType T = O - v0;
  InternalVecType E1 = v1 - v0;
  InternalVecType E2 = v2 - v0;
  InternalVecType P = Cross(dir, E2);
  InternalVecType Q = Cross(T, E1);
  InternalScalarType det = Dot(P, E1);

  if (det < 0.00000001) {
    return false;
  }

  InternalScalarType ndet = 1.0 / det;

  t = ndet * Dot(Q, E2);
  u = ndet * Dot(P, T);
  v = ndet * Dot(Q, dir);

  // Termination conditions
  if (u < 0 || u > 1) return false;
  if (v < 0 || u + v > 1) return false;
  if (t < ray.t_min || t > ray.t_max) return false;

  // Export related information
  // 1. If mesh->has_texture, calculate the texture coordinates
  // 2. If mesh->has_normal, calculate the normal
  // you don't have to consider this part.
  CalculateTriangleDifferentials(interaction,
      {static_cast<Float>(1 - u - v), static_cast<Float>(u),
          static_cast<Float>(v)},
      mesh, triangle_index);
  AssertNear(interaction.p, ray(t));
  assert(ray.withinTimeRange(t));
  ray.setTimeMax(static_cast<Float>(t));

  return true;
}

void Accel::setTriangleMesh(const ref<TriangleMeshResource> &mesh) {
  // Build the bounding box
  AABB bound(Vec3f(Float_INF, Float_INF, Float_INF),
      Vec3f(Float_MINUS_INF, Float_MINUS_INF, Float_MINUS_INF));
  for (auto &vertex : mesh->vertices) {
    bound.low_bnd   = Min(bound.low_bnd, vertex);
    bound.upper_bnd = Max(bound.upper_bnd, vertex);
  }

  this->mesh  = mesh;   // set the pointer
  this->bound = bound;  // set the bounding box
}

void Accel::build() {}

AABB Accel::getBound() const {
  return bound;
}

bool Accel::intersect(Ray &ray, SurfaceInteraction &interaction) const {
  bool success = false;
  for (int i = 0; i < mesh->v_indices.size() / 3; i++)
    success |= TriangleIntersect(ray, i, mesh, interaction);
  return success;
}

RDR_NAMESPACE_END
