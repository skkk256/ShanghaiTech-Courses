//#include <omp.h>
#include "optics_data.hpp"
#include "volume_renderer.hpp"
#include "config.hpp"


extern Config conf;

/**
 * VolumeRenderer class
 */

VolumeRenderer::VolumeRenderer()
    : camera(NULL)
    , classifier(NULL)
    , geom(NULL)
{
}

void VolumeRenderer::setCamera(Camera* cam)
{
    camera = cam;
}

void VolumeRenderer::addLight(Light* li)
{
    lights.push_back(li);
}

void VolumeRenderer::setGeometry(ImplicitGeometry* implicit_geom)
{
    geom = implicit_geom;
}

void VolumeRenderer::setClassifier(Classifier* cls)
{
    classifier = cls;
}

void VolumeRenderer::renderFrontToBack()
{
    int res_x = camera->getFilm().resolution.x();
    int res_y = camera->getFilm().resolution.y();
    float dt = conf.step_len;

#ifndef NO_OMP
    #pragma omp parallel for
#endif
    for (int i = 0; i < res_x * res_y; ++i)
    {
        int dy = i / (float)res_x;
        int dx = i - dy * (float)res_x;
        Ray ray = camera->generateRay((float)dx, (float)dy);
        Eigen::Vector3f color(0, 0, 0);
        Eigen::Vector3f alpha(0, 0, 0);
        // TODO
        if (geom->bboxRayIntersection(ray, ray.range_min, ray.range_max))
        {
          for (float t = ray.range_min; t < ray.range_max; t += dt) {
            Eigen::Vector3f pos = ray.origin + t * ray.direction;
            VolumePointData v_data;
            v_data.position = pos;
            v_data.value = geom->getValue(pos);
            if (v_data.value < -0.5f || v_data.value > 0.5f) {
                continue;
            }
            v_data.gradient = geom->computeGradient(pos);

            OpticsData optics =
                classifier->transfer(v_data, camera, lights, dt);

            // Front-to-back compositing
            color = color + (Eigen::Vector3f::Ones() - alpha)
                                .cwiseProduct(optics.getColor());
            alpha = alpha + (Eigen::Vector3f::Ones() - alpha)
                                .cwiseProduct(optics.transparency);
            //                if (i == 0) {
            //                  std::cout << "color " << color.x() << " " << color.y() << " "
            //                            << color.z() << "\n";
            //                  std::cout << "alpha " << alpha.x() << " " << alpha.y() << " "
            //                            << alpha.z() << "\n";
            //                }
            if (alpha[0] >= 1 && alpha[1] >= 1 && alpha[2] >= 1) {
              break;
            }
          }
        }
        camera->setPixel(dx, dy, color);
    }
};
