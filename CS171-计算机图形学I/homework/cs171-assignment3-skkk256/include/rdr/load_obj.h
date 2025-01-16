#ifndef __LOAD_OBJ_H__
#define __LOAD_OBJ_H__

#include <string>

#include "rdr/rdr.h"

RDR_NAMESPACE_BEGIN

bool LoadObj(const std::string &path, vector<Vec3f> &vertices,
    vector<Vec3f> &normals, vector<Vec2f> &texture_coordinates,
    vector<uint32_t> &v_index, vector<uint32_t> &n_index,
    vector<uint32_t> &t_index);

RDR_NAMESPACE_END

#endif
