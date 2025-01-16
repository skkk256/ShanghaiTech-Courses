#define TINYCOLORMAP_WITH_EIGEN
#include "tinycolormap.hpp"
#include "classifier.hpp"
#include "constant.hpp"
#include "utils.hpp"


/**
 * IsosurfaceClassifier class
 */

IsosurfaceClassifier::IsosurfaceClassifier(float isoval)
    : isovalue(isoval)
{
}

OpticsData IsosurfaceClassifier::transfer(VolumePointData v_data, Camera *cam, const std::vector<Light*>& lights, float dt) const
{
    OpticsData optics;
    // TODO
    float sigma = 100*dt;
    float c = 4.0f;
    float surface_isovalue = 0;
    float gradient_magnitude = v_data.gradient.norm();
    float distance = c*std::abs(v_data.value - surface_isovalue);
    float transparency = exp(-(distance * distance) / (2 * sigma * sigma))/sqrt(2 * PI * sigma * sigma);
//    if (v_data.value > -0.01 && v_data.value < 0.01) {
    if (transparency > 0.001) {
        Eigen::Vector3f normal = v_data.gradient.normalized();
        const tinycolormap::Color color_map = tinycolormap::GetColor((normal.x() + 1)/2.0, tinycolormap::ColormapType::Parula);
        optics.color = Eigen::Vector3f(0.03, 0.03, 0.03).cwiseProduct(Eigen::Vector3f(color_map.r(), color_map.g(), color_map.b()));

        for (auto light : lights) {
            Eigen::Vector3f color = light->getColor().cwiseProduct(Eigen::Vector3f(color_map.r(), color_map.g(), color_map.b()));
            Eigen::Vector3f light_dir =
                (light->getPosition() - v_data.position).normalized();
            Eigen::Vector3f view_dir =
                (cam->getPosition() - v_data.position).normalized();
            Eigen::Vector3f reflect =
                (2 * normal * (normal.dot(light_dir)) - light_dir).normalized();

    //        if (v_data.position.x() > -0.1 && v_data.position.x() < 0.1 && v_data.position.y() < 0 && v_data.position.z() > 0) {
    //          std::cout << "pos " << v_data.position.x() << " " << v_data.position.y() << " "
    //                    << v_data.position.z() << "\n";
    //          std::cout << normal.dot(light_dir) << "\n";
    //        }

            Eigen::Vector3f diffuse = std::max(0.f, normal.dot(light_dir)) * color;
            Eigen::Vector3f specular =
                std::pow(std::max(0.f, -reflect.dot(view_dir)), 10) * color;
            optics.color += 0.4*diffuse + 0.5*specular;
        }
        optics.color = transparency * optics.color;
        optics.transparency = transparency * Eigen::Vector3f(1, 1, 1);
//      optics.color = optics.color;
//      optics.transparency = Eigen::Vector3f(1, 1, 1);
    }
    else {
        optics.color = Eigen::Vector3f(0, 0, 0);
        optics.transparency = Eigen::Vector3f(0, 0, 0);
    }
    return optics;
}

