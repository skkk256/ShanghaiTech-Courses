#pragma once
#include "camera.hpp"
#include "classifier.hpp"
#include "light.hpp"
#include "implicit_geom.hpp"
#include <vector>


class VolumeRenderer
{
protected:
    Camera *camera;
    std::vector<Light*> lights;
    ImplicitGeometry* geom;
    Classifier* classifier;

public:
    VolumeRenderer();
    void setCamera(Camera* cam);
    void addLight(Light* li);
    void setGeometry(ImplicitGeometry* implicit_geom);
    void setClassifier(Classifier* cls);
    void renderFrontToBack();
};
