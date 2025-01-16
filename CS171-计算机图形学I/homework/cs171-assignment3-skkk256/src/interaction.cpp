#include "rdr/interaction.h"

#include "rdr/canary.h"
#include "rdr/math_aliases.h"
#include "rdr/math_utils.h"
#include "rdr/platform.h"
#include "rdr/shape.h"

RDR_NAMESPACE_BEGIN

void SurfaceInteraction::CalculateRayDifferentials(const DifferentialRay &ray) {
  AssertAllNormalized(
      ray.direction, ray.dx_direction, ray.dy_direction, normal);
  if (ray.has_differential) {
    /** Assume that two offset rays will both intersect with a very
     ** small plane which is normal \cdot (v - p) = 0.
     */
    auto d  = -Dot(normal, p);
    auto tx = (-Dot(normal, ray.dx_origin) - d) / Dot(normal, ray.dx_direction);
    if (!IsAllValid(tx)) goto fail;
    auto px = ray.dx_origin + tx * ray.dx_direction;
    auto ty = (-Dot(normal, ray.dy_origin) - d) / Dot(normal, ray.dy_direction);
    if (!IsAllValid(ty)) goto fail;
    auto py = ray.dy_origin + ty * ray.dy_direction;
    AssertAllValid(px, py);
    internal.dpdx = px - p;
    internal.dpdy = py - p;

    /** dpdx = dudx * dpdu + dvdx * dpdv
     ** dpdy = dudy * dpdu + dvdy * dpdv
     ** for each vector equation, we have three scalar equation, but only two
     ** scalar unknowns. Just solve them, we can get dudx, dvdx, dudy, dvdy.
     */

    /**
     * @brief Solve 2x2 linear system.
     */
    auto SolveLinearSystem2x2 = [](const Float A[2][2], const Float B[2],
                                    Float *x0, Float *x1) {
      Float det = A[0][0] * A[1][1] - A[0][1] * A[1][0];
      if (std::abs(det) < 1e-10f) return false;
      *x0 = (A[1][1] * B[0] - A[0][1] * B[1]) / det;
      *x1 = (A[0][0] * B[1] - A[1][0] * B[0]) / det;
      return !(std::isnan(*x0) || std::isnan(*x1));
    };

    /** choose what equation we will use. */
    /** ref:
     * https://pbr-book.org/3ed-2018/Texture/Sampling_and_Antialiasing#FindingtheTextureSamplingRate
     */
    int dim[2];
    if (std::abs(normal.x) > std::abs(normal.y) &&
        std::abs(normal.x) > std::abs(normal.z)) {
      dim[0] = 1;
      dim[1] = 2;
    } else if (std::abs(normal.y) > std::abs(normal.z)) {
      dim[0] = 0;
      dim[1] = 2;
    } else {
      dim[0] = 0;
      dim[1] = 1;
    }

    Float A[2][2] = {
        {dpdu[dim[0]], dpdv[dim[0]]},
        {dpdu[dim[1]], dpdv[dim[1]]}
    };
    Float Bx[2] = {px[dim[0]] - p[dim[0]], px[dim[1]] - p[dim[1]]};
    Float By[2] = {py[dim[0]] - p[dim[0]], py[dim[1]] - p[dim[1]]};
    if (!SolveLinearSystem2x2(A, Bx, &internal.dudx, &internal.dvdx))
      internal.dudx = internal.dvdx = 0;
    if (!SolveLinearSystem2x2(A, By, &internal.dudy, &internal.dvdy))
      internal.dudy = internal.dvdy = 0;

  } else {
fail:
    internal.dudx = internal.dvdx = 0;
    internal.dudy = internal.dvdy = 0;
    internal.dpdx = internal.dpdy = {0, 0, 0};
  }
}

void CalculateTriangleDifferentials(SurfaceInteraction &interaction,
    const Vec3f &b, const ref<TriangleMeshResource> &mesh,
    const uint32_t triangle_index) {
  AssertAllValid(b);

  Vec2f t[3];
  Vec3f n[3], p[3];

  {
    const auto &v = mesh->vertices;
    const Vec3u v_idx{&mesh->v_indices[3 * triangle_index]};
    assert(v_idx.x < mesh->vertices.size());
    assert(v_idx.y < mesh->vertices.size());
    assert(v_idx.z < mesh->vertices.size());

    p[0] = v[v_idx[0]];
    p[1] = v[v_idx[1]];
    p[2] = v[v_idx[2]];
  }

  if (mesh->has_texture) {
    const auto &tex = mesh->texture_coordinates;
    const Vec3u t_idx{&mesh->t_indices[3 * triangle_index]};
    assert(t_idx.x < mesh->texture_coordinates.size());
    assert(t_idx.y < mesh->texture_coordinates.size());
    assert(t_idx.z < mesh->texture_coordinates.size());

    t[0] = tex[t_idx[0]];
    t[1] = tex[t_idx[1]];
    t[2] = tex[t_idx[2]];
  } else {
    t[0] = {0, 0};
    t[1] = {1, 0};
    t[2] = {1, 1};
  }

  if (mesh->has_normal) {
    const auto &normals = mesh->normals;
    const Vec3u n_idx(&mesh->n_indices[3 * triangle_index]);
    assert(n_idx.x < mesh->normals.size());
    assert(n_idx.y < mesh->normals.size());
    assert(n_idx.z < mesh->normals.size());

    n[0] = normals[n_idx[0]];
    n[1] = normals[n_idx[1]];
    n[2] = normals[n_idx[2]];
  }

  /** === Differentiation process === */
  /** (u, v) = b_0 * t_0 + b_1 * t_1 + b_2 * t_2
   **        = b_0 * (t_0 - t_2) + b_1 * (t_1 - t_2) + t_2
   **        = b_0 * Dt02 + b_1 * Dt12 + t_2
   **
   ** Multiply inverse matrix, det = Dt02[0] * Dt12[1] - Dt02[1] * Dt12[0]
   **
   ** b_0    = {  Dt12[1] * u - Dt12[0] * v } / det
   ** b_1    = { -Dt02[1] * u + Dt02[0] * v } / det
   **
   ** db_0du =  Dt12[1] / det
   ** db_0dv = -Dt12[0] / det
   ** db_1du = -Dt02[1] / det
   ** db_1dv =  Dt02[0] / det
   **
   ** p      = b_0 * p_0 + b_1 * p_1 + b_2 * p_2
   **        = b_0 * (p_0 - p_2) + b_1 * (p_1 - p_2)
   **        = b_0 * Dp02 + b_1 * Dp12
   **
   ** dpdb_0 = Dp02
   ** dpdb_1 = Dp12
   **
   ** dpdu   = dpdb_0 * db_0du + dpdb_1 * db_1du
   **        = ( Dt12[1] * Dp02 - Dt02[1] * Dp12) / det
   **
   ** dpdv   = dpdb_0 * db_0dv + dpdb_1 * db_1dv
   **        = (-Dt12[0] * Dp02 + Dt02[0] * Dp12) / det
   */

  Vec3f dpdu, dpdv;
  Vec2f Dt02 = t[0] - t[2], Dt12 = t[1] - t[2];
  Vec3f Dp02 = p[0] - p[2], Dp12 = p[1] - p[2];
  Float determinant = Dt02[0] * Dt12[1] - Dt02[1] * Dt12[0];
  bool degenerateUV = std::abs(determinant) < 1e-8;
  if (!degenerateUV) {
    Float invdet = 1 / determinant;
    dpdu         = (Dt12[1] * Dp02 - Dt02[1] * Dp12) * invdet;
    dpdv         = (-Dt12[0] * Dp02 + Dt02[0] * Dp12) * invdet;
  }

  Vec3f intersect_p = b[0] * p[0] + b[1] * p[1] + b[2] * p[2];
  Vec3f intersect_n = Normalize(Cross(p[1] - p[0], p[2] - p[0]));
  Vec2f uv          = b[0] * t[0] + b[1] * t[1] + b[2] * t[2];

  if (degenerateUV || SquareNorm(Cross(dpdu, dpdv)) == 0) {
    // Handle zero determinant for triangle partial derivative matrix
    CoordinateSystemFromNormal(Normalize(intersect_n), dpdu, dpdv);
  }

  interaction.setDifferential(
      intersect_p, intersect_n, uv, dpdu, dpdv, {0, 0, 0}, {0, 0, 0});

  // Compute differentials and normal under shading geometry
  if (mesh->has_normal) {
    Vec3f shading_normal, shading_tangent, shading_bitangent;
    shading_normal = b[0] * n[0] + b[1] * n[1] + b[2] * n[2];
    if (SquareNorm(shading_normal) > 0)
      shading_normal = Normalize(shading_normal);
    else
      shading_normal = intersect_n;

    shading_tangent = Normalize(dpdu);

    shading_bitangent = Cross(shading_normal, shading_tangent);
    if (SquareNorm(shading_bitangent) > 0)
      shading_bitangent = Normalize(shading_bitangent);
    else
      CoordinateSystemFromNormal(
          shading_normal, shading_tangent, shading_bitangent);

    // the process of obtaining dndu, dndv
    // is absolutely the same with dpdu, dpdv
    Vec3f dndu, dndv;
    Vec2f Dt02 = t[0] - t[2], Dt12 = t[1] - t[2];
    Vec3f Dn02 = n[0] - n[2], Dn12 = n[1] - n[2];
    Float determinant = Dt02[0] * Dt12[1] - Dt02[1] * Dt12[0];
    bool degenerateUV = std::abs(determinant) < 1e-8;
    if (!degenerateUV) {
      Float invdet = 1 / determinant;
      dndu         = (Dt12[1] * Dn02 - Dt02[1] * Dn12) * invdet;
      dndv         = (-Dt12[0] * Dn02 + Dt02[0] * Dn12) * invdet;
    } else {
      Vec3f dn = Cross(n[1] - n[0], n[2] - n[0]);
      if (SquareNorm(dn) == 0)
        dndu = dndv = {0, 0, 0};
      else {
        CoordinateSystemFromNormal(dn, dndu, dndv);
      }
    }
    interaction.setShading(
        shading_normal, shading_tangent, shading_bitangent, dndu, dndv);
  }
}

RDR_NAMESPACE_END