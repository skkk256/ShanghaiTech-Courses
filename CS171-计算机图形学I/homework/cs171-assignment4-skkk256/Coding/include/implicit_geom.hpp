#pragma once
#include "Eigen/Dense"
#include "bbox.hpp"
#include "ray.hpp"


/**
 * A data structure containing information of each point in the volume
 */

struct VolumePointData
{
    float value;
    Eigen::Vector3f gradient;
    Eigen::Vector3f position;
};


/**
 * Base class of implicit geometries
 */

class ImplicitGeometry
{
protected:
    AABB bounding_box;

public:
    AABB getBBox() const;
    /** Return the function/field value at the point p */
    virtual float getValue(Eigen::Vector3f p) = 0;
    /** Compute the gradient at the point p */
    virtual Eigen::Vector3f computeGradient(Eigen::Vector3f p) = 0;
    /** Check the ray-bbox intersection
     * the entry point will be set to t_entry
     * the exit point will be set to t_exit
     */
    virtual bool bboxRayIntersection(Ray r, float& t_entry, float& t_exit) const;
};


/**
 * Surface of genus 2
 */

class GenusTwoSurface : public ImplicitGeometry
{
public:
    GenusTwoSurface(Eigen::Vector3f pos, Eigen::Vector3f range);
    virtual float getValue(Eigen::Vector3f p);
    virtual Eigen::Vector3f computeGradient(Eigen::Vector3f p);
};


/**
 * Surface of wineglass
 */

class WineGlassSurface : public ImplicitGeometry
{
public:
    WineGlassSurface(Eigen::Vector3f pos, Eigen::Vector3f range);
    virtual float getValue(Eigen::Vector3f p);
    virtual Eigen::Vector3f computeGradient(Eigen::Vector3f p);
};


/**
 * Porous surface
 */

class PorousSurface : public ImplicitGeometry
{
public:
    PorousSurface(Eigen::Vector3f pos, Eigen::Vector3f range);
    virtual float getValue(Eigen::Vector3f p);
    virtual Eigen::Vector3f computeGradient(Eigen::Vector3f p);
};

class TwoObeject : public ImplicitGeometry
{
  public:
    TwoObeject(Eigen::Vector3f pos, Eigen::Vector3f range);
    virtual float getValue(Eigen::Vector3f p);
    virtual Eigen::Vector3f computeGradient(Eigen::Vector3f p);
};
