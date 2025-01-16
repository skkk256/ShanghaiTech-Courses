#pragma once
#include <vector>
#include "Eigen/Dense"
#include "camera.hpp"
#include "light.hpp"
#include "optics_data.hpp"
#include "implicit_geom.hpp"


/**
 * Base class of classifiers 
 */

class Classifier
{
public:
    virtual OpticsData transfer(VolumePointData v_data, Camera *cam, const std::vector<Light*>& lights, float dt) const = 0;
};


/**
 * The classifier for visualizing isosurfaces
 */

class IsosurfaceClassifier : public Classifier
{
protected:
    float isovalue;

public:
    IsosurfaceClassifier(float isoval);
    /**
     * Transfer function
     * @param v_data the volume data at the given point
     * @param cam    the camera used to compute Phong shading
     * @param lights a set of lights used to compute Phong shading
     * @param dt     the sampling step length
     */
    virtual OpticsData transfer(VolumePointData v_data, Camera *cam, const std::vector<Light*>& lights, float dt) const;
};