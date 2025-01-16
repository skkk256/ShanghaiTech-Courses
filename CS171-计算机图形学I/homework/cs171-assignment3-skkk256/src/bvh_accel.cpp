#include "rdr/bvh_accel.h"

#ifdef USE_EMBREE
#include <embree4/rtcore_buffer.h>
#include <embree4/rtcore_common.h>
#include <embree4/rtcore_device.h>
#include <embree4/rtcore_geometry.h>
#include <embree4/rtcore_scene.h>
#endif

#include <cstdlib>

#include "rdr/interaction.h"
#include "rdr/ray.h"
#include "rdr/shape.h"

RDR_NAMESPACE_BEGIN

void BVHAccel::setTriangleMesh(const ref<TriangleMeshResource> &mesh) {
  // extract information from mesh
  const uint32_t &num_triangles = mesh->v_indices.size() / 3;
  assert(mesh->v_indices.size() % 3 == 0);
  for (uint32_t i = 0; i < num_triangles; ++i)
    triangle_tree.push_back(detail_::Triangle(i, mesh));
}

void BVHAccel::build() {
  triangle_tree.build();
}

AABB BVHAccel::getBound() const {
  return triangle_tree.getAABB();
}

bool BVHAccel::intersect(Ray &ray, SurfaceInteraction &interaction) const {
  bool intersected = triangle_tree.intersect(ray,
      [&interaction](Ray &local_ray, const detail_::Triangle &triangle)
          -> bool { return triangle.intersect(local_ray, interaction); });
  return intersected;
}

#ifdef USE_EMBREE
ExternalBVHAccel::ExternalBVHAccel() {
  // Initialize Embree
  device = rtcNewDevice(nullptr);
  if (!device) Exception_("Cannot create Embree device");
  rtcSetDeviceErrorFunction(
      device,
      [](void *userPtr, enum RTCError error, const char *str) {
        Exception_("[Embree] Embree error: {}: {}", error, str);
      },
      nullptr);
  scene = rtcNewScene(device);
  geom  = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);
}

ExternalBVHAccel::~ExternalBVHAccel() {
  rtcReleaseGeometry(geom);
  rtcReleaseScene(scene);
  rtcReleaseDevice(device);
}

void ExternalBVHAccel::setTriangleMesh(const ref<TriangleMeshResource> &mesh) {
  // Initialize and set buffers
  float *vertices =
      static_cast<float *>(rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX,
          0, RTC_FORMAT_FLOAT3, sizeof(Vec3f), mesh->vertices.size()));
  uint32_t *indices = static_cast<uint32_t *>(
      rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3,
          sizeof(uint32_t) * 3, mesh->v_indices.size() / 3));
#if false
  // There is some problems in the legacy loader;
  rtcSetGeometryVertexAttributeCount(geom, 1);
  float *normals = static_cast<float *>(rtcSetNewGeometryBuffer(
      geom, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 0, RTC_FORMAT_FLOAT3,
      sizeof(Vec3f), mesh->vertices.size()));
#endif
  std::memcpy(
      vertices, mesh->vertices.data(), sizeof(Vec3f) * mesh->vertices.size());
  std::memcpy(indices, mesh->v_indices.data(),
      sizeof(uint32_t) * mesh->v_indices.size());

  this->mesh = mesh;
}

void ExternalBVHAccel::build() {
  rtcSetGeometryBuildQuality(geom, RTC_BUILD_QUALITY_HIGH);
  rtcCommitGeometry(geom);
  geomId = rtcAttachGeometry(scene, geom);
  rtcCommitScene(scene);
}

AABB ExternalBVHAccel::getBound() const {
#if !defined(_WIN32)
  auto *bounds =
      static_cast<RTCBounds *>(std::aligned_alloc(16, sizeof(RTCBounds)));
#else
  auto *bounds =
      static_cast<RTCBounds *>(_aligned_malloc(sizeof(RTCBounds), 16));
#endif

  rtcGetSceneBounds(scene, bounds);
  AABB result(Vec3f(bounds->lower_x, bounds->lower_y, bounds->lower_z),
      Vec3f(bounds->upper_x, bounds->upper_y, bounds->upper_z));
  std::free(bounds);
  return result;
}

bool ExternalBVHAccel::intersect(
    Ray &ray, SurfaceInteraction &interaction) const {
  // initialize rayhit struct
  RTCRayHit rayhit;
  rayhit.ray.org_x     = ray.origin.x;
  rayhit.ray.org_y     = ray.origin.y;
  rayhit.ray.org_z     = ray.origin.z;
  rayhit.ray.dir_x     = ray.direction.x;
  rayhit.ray.dir_y     = ray.direction.y;
  rayhit.ray.dir_z     = ray.direction.z;
  rayhit.ray.tnear     = ray.t_min;
  rayhit.ray.tfar      = ray.t_max;
  rayhit.ray.mask      = -1;
  rayhit.ray.flags     = 0;
  rayhit.hit.geomID    = RTC_INVALID_GEOMETRY_ID;
  rayhit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;

  AssertAllNormalized(ray.direction);
  rtcIntersect1(scene, &rayhit);

  if (rayhit.hit.geomID == RTC_INVALID_GEOMETRY_ID) return false;
  assert(rayhit.hit.geomID == geomId);

  // Intersect function should consider ray's timerange
  if (!ray.withinTimeRange(rayhit.ray.tfar)) return false;

  const Float &t  = rayhit.ray.tfar;
  const Vec2f &uv = Vec2f(rayhit.hit.u, rayhit.hit.v);

  const uint32_t triangle_index = rayhit.hit.primID;
  CalculateTriangleDifferentials(
      interaction, {1 - uv[0] - uv[1], uv[0], uv[1]}, mesh, triangle_index);
  ray.setTimeMax(rayhit.ray.tfar);
  return true;
}
#endif  // USE_EMBREE

RDR_NAMESPACE_END
