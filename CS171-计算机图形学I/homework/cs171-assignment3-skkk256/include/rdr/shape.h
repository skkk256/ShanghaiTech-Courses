#ifndef __SHAPE_H__
#define __SHAPE_H__

#include <memory>

#include "rdr/rdr.h"

RDR_NAMESPACE_BEGIN

/**
 * @brief Shape is class for purely geometric properties, i.e. not for material.
 * It is important to decouple them for more efficiency.
 * @see Primitive for aggregate of geometric and material properties.
 */
class Shape : public ConfigurableObject {
public:
  ~Shape() override = default;

  // ++ Required by ConfigurableObject
  Shape(const Properties &props) : ConfigurableObject(props) {}
  // --

  /// Intersect ray with shape. If hit is found, return true and fill
  /// the SurfaceInteraction. Else, **return false and do nothing**, i.e.
  /// Surface Interaction will not be modified. This specification is important
  /// for correctness. Note that interaction.dist is taken into account.
  virtual bool intersect(Ray &ray, SurfaceInteraction &interaction) const = 0;

  /// Calculate the surface area of the shape to calculate PDF.
  virtual Float area() const = 0;

  /// Given a random sample in [0,1]^2, return a point on the shape and its PDF.
  virtual SurfaceInteraction sample(Sampler &sampler) const = 0;

  /// Return the bounding box of the shape.
  virtual AABB getBound() const = 0;

  /// Return the PDF of the given Interaction on the shape with a certain
  /// measure.
  virtual Float pdf(const SurfaceInteraction &interaction) const = 0;
};

class Sphere final : public Shape {
public:
  ~Sphere() override = default;

  // ++ Required by ConfigurableObject
  Sphere(const Properties &props);
  // --

  /// @see Shape::intersect
  bool intersect(Ray &ray, SurfaceInteraction &interaction) const override;

  /// @see Shape::area
  Float area() const override;

  /// @see Shape::sample
  SurfaceInteraction sample(Sampler &sampler) const override;

  /// @see Shape::getBound
  AABB getBound() const override;

  /// @see Shape::pdf
  Float pdf(const SurfaceInteraction &interaction) const override;

private:
  Vec3f center;
  Float radius;
};

/**
 * @brief Decouple the binary format of triangle mesh from the implementation.
 */
struct TriangleMeshResource {
  bool has_normal{false};
  bool has_texture{false};
  vector<Vec3f> vertices;
  vector<Vec3f> normals;
  vector<Vec2f> texture_coordinates;
  vector<uint32_t> v_indices;
  vector<uint32_t> n_indices;
  vector<uint32_t> t_indices;

  RDR_FORCEINLINE Vec3f getVertex(std::size_t i) const {
    assert(i < v_indices.size());
    return vertices[v_indices[i]];
  }
};

/**
 * @brief Triangle mesh is a collection of triangles. Triangles are not defined
 * separately because we don't need to (consider why?). This way we can decouple
 * the binary format of triangles (which can be complex) and focus on the
 * implementation. For example, we can readily implement Struct of Array
 * triangles on this structure without extra definitions. We can also wrap other
 * acceleration structures like Embree.
 */
class TriangleMesh final : public Shape {
public:
  ~TriangleMesh() override = default;

  // ++ Required by ConfigurableObject
  TriangleMesh(const Properties &props);
  // --

  /// @see Shape::intersect
  bool intersect(Ray &ray, SurfaceInteraction &interaction) const override;

  /// @see Shape::area
  Float area() const override;

  /// @see Shape::sample
  SurfaceInteraction sample(Sampler &sampler) const override;

  /// @see Shape::getBound
  AABB getBound() const override;

  /// @see Shape::pdf
  Float pdf(const SurfaceInteraction &interaction) const override;

protected:
  ref<Accel> accel;  //<! Any acceleration structure. Will not fully
                     // fill the interaction structure.
  ref<TriangleMeshResource> mesh;  //<! Triangle mesh data. Should be
                                   // defined as pointer for aggregation

  /// sampling-related members.
  ref<Distribution1D> dist;  //<! Distribution of areas.
  vector<Float> areas;       //<! Area of each triangle. Will be
                             // calculated on construction.
  Float total_area{};        //<! Total area of the mesh.
};

RDR_REGISTER_CLASS(Sphere)
RDR_REGISTER_CLASS(TriangleMesh)

RDR_REGISTER_FACTORY(Shape, [](const Properties &props) -> Shape * {
  auto type = props.getProperty<std::string>("type");
  if (type == "mesh") {
    return Memory::alloc<TriangleMesh>(props);
  } else if (type == "sphere") {
    return Memory::alloc<Sphere>(props);
  } else {
    Exception_("Shape type {} not supported", type);
  }

  return nullptr;
})

RDR_NAMESPACE_END

#endif
