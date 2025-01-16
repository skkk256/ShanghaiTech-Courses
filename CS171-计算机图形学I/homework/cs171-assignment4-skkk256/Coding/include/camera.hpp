#pragma once
#include "Eigen/Dense"
#include "film.hpp"
#include "ray.hpp"


/**
 * Pin-hole camera model
 */

class Camera
{
protected:
    Eigen::Vector3f position;
    Eigen::Vector3f forward;
    Eigen::Vector3f right;
    Eigen::Vector3f up;
    float vertical_fov;
    Film film;

public:
    Camera(const Eigen::Vector3f& pos, float v_fov, const Eigen::Vector2i& film_res);
	void lookAt(const Eigen::Vector3f& look_at, const Eigen::Vector3f& ref_up);
    Ray generateRay(float dx, float dy) const;
    void setPixel(int dx, int dy, Eigen::Vector3f value);
    const Film& getFilm() const;
    Eigen::Vector3f getPosition() const;
};
