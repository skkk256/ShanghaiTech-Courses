#pragma once
#include "bbox.hpp"


/**
 * AABB class
 */

AABB::AABB()
    : lb(0, 0, 0)
    , ub(0, 0, 0)
{
}
    
AABB::AABB(float lb_x, float lb_y, float lb_z, float ub_x, float ub_y, float ub_z)
{
    lb = Eigen::Vector3f(lb_x, lb_y, lb_z);
    ub = Eigen::Vector3f(ub_x, ub_y, ub_z);
}

AABB::AABB(Eigen::Vector3f lb, Eigen::Vector3f ub)
    : lb(lb)
    , ub(ub)
{
}

AABB::AABB(const Eigen::Vector3f& pos, float radius)
{
    Eigen::Vector3f r(radius, radius, radius);
    lb = pos - r;
    ub = pos + r;
}

AABB::AABB(const Eigen::Vector3f& v1, const Eigen::Vector3f& v2, const Eigen::Vector3f& v3)
{
    lb = v1.cwiseMin(v2).cwiseMin(v3);
    ub = v1.cwiseMax(v2).cwiseMax(v3);
}

AABB::AABB(const AABB& a, const AABB& b)
{
    lb = Eigen::Vector3f(a.lb.cwiseMin(b.lb));
    ub = Eigen::Vector3f(a.ub.cwiseMax(b.ub));
}

Eigen::Vector3f AABB::getCenter() const
{
    return (lb + ub) / 2;
}

Eigen::Vector3f AABB::getSize() const
{
    return ub - lb;
}


float AABB::getDist(int c) const
{
    return ub[c] - lb[c];
}

float AABB::getVolume() const
{
    return getDist(2) * getDist(1) * getDist(0);
}

bool AABB::isOverlap(const AABB& a) const
{
    return ((a.lb[0] >= this->lb[0] && a.lb[0] <= this->ub[0]) || (this->lb[0] >= a.lb[0] && this->lb[0] <= a.ub[0])) &&
        ((a.lb[1] >= this->lb[1] && a.lb[1] <= this->ub[1]) || (this->lb[1] >= a.lb[1] && this->lb[1] <= a.ub[1])) &&
        ((a.lb[2] >= this->lb[2] && a.lb[2] <= this->ub[2]) || (this->lb[2] >= a.lb[2] && this->lb[2] <= a.ub[2]));

}

float AABB::diagonalLength() const
{
    return (ub - lb).norm();
}

bool AABB::rayIntersection(const Ray& ray, float& t_in, float& t_out) const
{
    float dir_frac_x = (ray.direction[0] == 0.0) ? 1.0e32 : 1.0f / ray.direction[0];
    float dir_frac_y = (ray.direction[1] == 0.0) ? 1.0e32 : 1.0f / ray.direction[1];
    float dir_frac_z = (ray.direction[2] == 0.0) ? 1.0e32 : 1.0f / ray.direction[2];

    float tx1 = (lb[0] - ray.origin[0]) * dir_frac_x;
    float tx2 = (ub[0] - ray.origin[0]) * dir_frac_x;
    float ty1 = (lb[1] - ray.origin[1]) * dir_frac_y;
    float ty2 = (ub[1] - ray.origin[1]) * dir_frac_y;
    float tz1 = (lb[2] - ray.origin[2]) * dir_frac_z;
    float tz2 = (ub[2] - ray.origin[2]) * dir_frac_z;

    t_in = std::max(std::max(std::min(tx1, tx2), std::min(ty1, ty2)), std::min(tz1, tz2));
    t_out = std::min(std::min(std::max(tx1, tx2), std::max(ty1, ty2)), std::max(tz1, tz2));

    /* When t_out < 0 and the ray is intersecting with AABB, the whole AABB is behind us */
    if (t_out < 0)
    {
        return false;
    }

    return t_out >= t_in;
}
