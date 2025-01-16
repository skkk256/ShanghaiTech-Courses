#pragma once
#include <Eigen/Dense>


/**
 * Point lights
 */

class Light
{
protected:
	Eigen::Vector3f position;
	Eigen::Vector3f color;

public:
	Light(Eigen::Vector3f pos, Eigen::Vector3f rgb);
	virtual Eigen::Vector3f getPosition() const;
	virtual Eigen::Vector3f getColor() const;
};
