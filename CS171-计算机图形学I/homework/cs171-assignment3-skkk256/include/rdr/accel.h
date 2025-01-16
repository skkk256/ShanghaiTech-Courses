#ifndef __ACCEL_H__
#define __ACCEL_H__

#include "rdr/rdr.h"

RDR_NAMESPACE_BEGIN

// Forward declaration
bool TriangleIntersect(Ray &ray, const uint32_t &triangle_index,
    const ref<TriangleMeshResource> &mesh, SurfaceInteraction &interaction);

/**
 * @brief The bounding box
 */
template <typename PointType_>
struct TAABB {
  using PointType = PointType_;

  // The minimum and maximum coordinate for the AABB
  PointType low_bnd{Float_INF};
  PointType upper_bnd{Float_MINUS_INF};

  /// test intersection with given ray.
  /// ray distance of entrance and exit point are recorded in t_in and t_out
  TAABB() = default;
  TAABB(const PointType &low, const PointType &upper)
      : low_bnd(low), upper_bnd(upper) {}

  /// Construct an AABB from three vertices of a triangle.
  TAABB(const PointType &v1, const PointType &v2, const PointType &v3)
      : low_bnd(Min(v1, v2, v3)), upper_bnd(Max(v1, v2, v3)) {}

  /// Construct AABB by merging two AABBs
  TAABB(const TAABB &a, const TAABB &b)
      : low_bnd(Min(a.low_bnd, b.low_bnd)),
        upper_bnd(Max(a.upper_bnd, b.upper_bnd)) {}

  /// Construct AABB by adding a vertex
  TAABB(const TAABB &a, const PointType &v)
      : low_bnd(Min(a.low_bnd, v)), upper_bnd(Max(a.upper_bnd, v)) {}

  /// Get the AABB center
  PointType getCenter() const { return (low_bnd + upper_bnd) / 2; }

  /// Get the extent of the AABB
  PointType getExtent() const { return upper_bnd - low_bnd; }

  /// Get the volume of the AABB
  Float getVolume() const { return ReduceProduct(getExtent()); }

  /// Get the length of a specified side on the AABB
  Float getDist(int dim) const { return upper_bnd[dim] - low_bnd[dim]; }

  std::string toString() const {
    std::ostringstream ss;
    ss << "TAABB[\n"
       << format("  low_bnd = {}\n", low_bnd)
       << format("  upper_bnd = {}\n", upper_bnd) << "]";
    return ss.str();
  }

  /// In-place union with another AABB
  void unionWith(const TAABB &other) {
    low_bnd   = Min(low_bnd, other.low_bnd);
    upper_bnd = Max(upper_bnd, other.upper_bnd);
  }

  /// In-place union with another position
  void unionWith(const PointType &v) {
    low_bnd   = Min(low_bnd, v);
    upper_bnd = Max(upper_bnd, v);
  }

  /// Check whether the AABB is valid
  bool isValid() const {
    return IsAllUnaryElementwise<detail_::IsNonNegative>(upper_bnd - low_bnd);
  }
};

struct AABB : public TAABB<Vec3f> {
  using TAABB::TAABB;

  /// \brief Perform a tMin/tMax aware intersection test with a ray.
  ///
  /// This function is quite special.. It will not modify the ray.tMax but
  /// return the result in t_in and t_out.
  bool intersect(const Ray &ray, Float *t_in, Float *t_out) const;

  /// Check whether the AABB is overlapping with another AABB
  bool isOverlap(const AABB &other) const;

  bool isInside(const Vec3f &v) const {
    return v.x >= low_bnd.x && v.x <= upper_bnd.x && v.y >= low_bnd.y &&
           v.y <= upper_bnd.y && v.z >= low_bnd.z && v.z <= upper_bnd.z;
  }

  Float getSurfaceArea() const {
    auto extent = Max(getExtent(), Vec3f(0.0));
    return (extent.x * extent.y + extent.y * extent.z + extent.z * extent.x) *
           2;
  }
};

/**
 * @brief Acceleration structure for ray-geometry intersection. Support triangle
 * mesh only. This is the base class for all acceleration structures such as
 * BVH/KD-Tree/Octree.
 */
class Accel {
public:
  Accel()          = default;
  virtual ~Accel() = default;

  /// Set the triangle mesh to be intersected.
  virtual void setTriangleMesh(const ref<TriangleMeshResource> &mesh);

  /// Build the acceleration structure.
  virtual void build();

  /// Return the bounding box of the structure in its space (not necessarily
  /// world space)
  virtual AABB getBound() const;

  /**
   * @brief Intersect ray with the acceleration structure. The specification is,
   * if there's a hit, modify the interaction's terms except for material. Else
   * do nothing.
   *
   * @param ray
   * @param interaction
   * @return bool for whether there's a hit.
   */
  virtual bool intersect(Ray &ray, SurfaceInteraction &interaction) const;

protected:
  ref<TriangleMeshResource>
      mesh{};  //<! The triangle mesh's underlying data to be intersected.
  AABB bound;  //<! The bounding box of the structure in its space.
};

RDR_NAMESPACE_END

#endif
