#include "rdr/load_obj.h"

#include <cstdint>

#include "rdr/platform.h"
#include "rdr/shape.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

RDR_NAMESPACE_BEGIN

bool LoadObj(const std::string &path, vector<Vec3f> &vertices,
    vector<Vec3f> &normals, vector<Vec2f> &texture_coordinates,
    vector<uint32_t> &v_index, vector<uint32_t> &n_index,
    vector<uint32_t> &t_index) {
  Info_("Loading model {}", path);

  tinyobj::ObjReaderConfig readerConfig;
  tinyobj::ObjReader reader;
  readerConfig.triangulation_method = "earcut";

  if (!reader.ParseFromFile(path, readerConfig)) {
    if (!reader.Error().empty())
      Exception_("TinyObjReader: {}", reader.Error());
  }

  if (!reader.Warning().empty()) {
    Warn_("TinyObjReader: {}", reader.Warning());
  }

  auto &attrib    = reader.GetAttrib();
  auto &shapes    = reader.GetShapes();
  auto &materials = reader.GetMaterials();

  for (size_t i = 0; i < attrib.vertices.size(); i += 3) {
    vertices.emplace_back(
        attrib.vertices[i], attrib.vertices[i + 1], attrib.vertices[i + 2]);
  }

  for (size_t i = 0; i < attrib.normals.size(); i += 3) {
    normals.emplace_back(
        attrib.normals[i], attrib.normals[i + 1], attrib.normals[i + 2]);
  }

  for (size_t i = 0; i < attrib.texcoords.size(); i += 2)
    texture_coordinates.emplace_back(
        attrib.texcoords[i], attrib.texcoords[i + 1]);

  // Loop over shapes
  for (size_t s = 0; s < shapes.size(); s++) {
    // Loop over faces(polygon)
    size_t index_offset = 0;
    for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
      auto fv = size_t(shapes[s].mesh.num_face_vertices[f]);

      // Loop over vertices in the face.
      for (size_t v = 0; v < fv; v++) {
        tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
        v_index.push_back(idx.vertex_index);
        n_index.push_back(idx.normal_index);
        t_index.push_back(idx.texcoord_index);
      }

      index_offset += fv;
    }
  }

  Info_(" # vertices:            {}", attrib.vertices.size() / 3);
  Info_(" # normals:             {}", attrib.normals.size() / 3);
  Info_(" # texture coordinates: {}", attrib.texcoords.size() / 2);
  Info_(" # faces:               {}", v_index.size() / 3);
  return true;
}

RDR_NAMESPACE_END
