#ifndef __BVH_ACCEL__
#define __BVH_ACCEL__

#include "rdr/shape.h"
#ifdef USE_EMBREE
#include <embree4/rtcore.h>

#include <utility>
#endif

#include "rdr/accel.h"
#include "rdr/bvh_tree.h"
#include "rdr/rdr.h"
#include "rdr/shape.h"

RDR_NAMESPACE_BEGIN

namespace detail_ {
/**
 * @brief In general, this class is not needed but costly when we have a large
 * number of Triangles. But to make our BVHTree works not only as a
 * primitive-bvh, this is implemented to test and serve as an inefficient
 * reference. This is not exposed to factory.
 */
class Triangle {
public:
  Triangle(int triangle_index, const ref<TriangleMeshResource> &in_mesh)
      : triangle_index(triangle_index), mesh(in_mesh) {
    assert(in_mesh.get() != nullptr);
  }

  bool intersect(Ray &ray, SurfaceInteraction &interaction) const {
    return TriangleIntersect(ray, triangle_index, mesh.get(), interaction);
  }

  AABB getBound() const {
    assert(mesh.get() != nullptr);
    const auto &v0 = mesh->getVertex(triangle_index * 3 + 0);
    const auto &v1 = mesh->getVertex(triangle_index * 3 + 1);
    const auto &v2 = mesh->getVertex(triangle_index * 3 + 2);
    return {v0, v1, v2};
  }

private:
  // The necessary information to perform TriangleIntersect
  // clang-format off
  int triangle_index;
  ref<TriangleMeshResource> mesh{nullptr};
  // clang-format on
};

class BVHTriangleNode final : BVHNodeInterface<Triangle> {
  using DataType = Triangle;

public:
  BVHTriangleNode(DataType data) : data(std::move(data)) {}

  // Implement the interface
  AABB getAABB() const override { return data.getBound(); }
  const DataType &getData() const override { return data; }

private:
  DataType data;
};
}  // namespace detail_

/// A somehow very inefficient BVH implementation based on the general BVH class
class BVHAccel final : public Accel {
public:
  ~BVHAccel() override = default;

  /// @see Accel::setTriangleMesh
  void setTriangleMesh(const ref<TriangleMeshResource> &mesh) override;

  /// @see Accel::build
  void build() override;

  /// @see Accel::getBound
  AABB getBound() const override;

  /// @see Accel::intersect
  bool intersect(Ray &ray, SurfaceInteraction &interaction) const override;

private:
  BVHTree<detail_::BVHTriangleNode> triangle_tree;
};

#ifdef USE_EMBREE
class ExternalBVHAccel final : public Accel {
public:
  ExternalBVHAccel();
  ~ExternalBVHAccel() override;

  /// @see Accel::setTriangleMesh
  void setTriangleMesh(const ref<TriangleMeshResource> &mesh) override;

  /// @see Accel::build
  void build() override;

  /// @see Accel::getBound
  AABB getBound() const override;

  /// @see Accel::intersect
  bool intersect(Ray &ray, SurfaceInteraction &interaction) const override;

private:
  /// Embree properties
  RTCDevice device;
  RTCScene scene;
  RTCGeometry geom;
  uint32_t geomId;
  ref<TriangleMeshResource> mesh;
};
#endif  // USE_EMBREE

RDR_NAMESPACE_END

#endif
