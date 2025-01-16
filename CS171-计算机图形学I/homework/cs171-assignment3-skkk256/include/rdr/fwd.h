/**
 * @file fwd.h
 * @author ShanghaiTech CS171 TAs
 * @brief Forward definitions.
 * @version 0.1
 * @date 2023-04-12
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef __FWD_H__
#define __FWD_H__

#include "rdr/platform.h"

RDR_NAMESPACE_BEGIN

/// Aggregations
class Scene;
struct CrossConfigurationContext;

/// Peripherals
class Light;
class AreaLight;
class InfiniteAreaLight;
class BSDF;
class Camera;
class ReconstructionFilter;
class Film;  // for saving
class FilmBlockView;
struct Ray;
struct DifferentialRay;
struct SurfaceInteraction;

template <typename _PointType>
struct TAABB;
struct AABB;

/// Shapes
class Shape;
class TriangleMesh;

/// Scene Objects
class Accel;
class Primitive;
struct TriangleMeshResource;
#ifdef USE_EMBREE
class ExternalBvhAccel;
#endif

/// KDTree
template <typename PointType_, typename DataType_>
struct KDNode;
template <typename NodeType_, typename AABBType_>
class KDTree;

/// Resource Management
class Properties;
template <typename T>
class BaseFactory;

/// Integrators
class Integrator;
class PathIntegrator;
class PhotonMappingIntegratorBase;
class PhotonMappingIntegrator;
class StochasticProgressivePhotonMappingIntegrator;

/// Texture
class Texture;  // for loading
class ConstantTexture;
class ImageTexture;
class MIPMap;

RDR_NAMESPACE_END

#endif
