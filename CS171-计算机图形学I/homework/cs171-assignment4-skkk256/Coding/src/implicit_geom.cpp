#include <cmath>
#include "implicit_geom.hpp"
#include "constant.hpp"
#include "utils.hpp"


/**
 * ImplicitGeometry class
 */

AABB ImplicitGeometry::getBBox() const
{
    return bounding_box;
}

bool ImplicitGeometry::bboxRayIntersection(Ray r, float &t_entry, float &t_exit) const
{
    return bounding_box.rayIntersection(r, t_entry, t_exit);
}


/**
 * GenusTwoSurface class
 */

GenusTwoSurface::GenusTwoSurface(Eigen::Vector3f pos, Eigen::Vector3f range)
{
    bounding_box = AABB(pos - range / 2.0, pos + range / 2.0);
}

float GenusTwoSurface::getValue(Eigen::Vector3f p)
{
    float x = p.x();
    float y = p.y();
    float z = p.z();

    return 2 * y * (y * y - 3 * x * x) * (1 - z * z) + pow(x * x + y * y, 2) - (9 * z * z - 1) * (1 - z * z);
}

Eigen::Vector3f GenusTwoSurface::computeGradient(Eigen::Vector3f p)
{
    // TODO
    float x = p.x();
    float y = p.y();
    float z = p.z();

    float df_dx = 4.0f*(pow(x, 3) + x*y*(y+3*pow(z, 2)-3));
    float df_dy = pow(x, 2) * (4*y + 6*pow(z, 2) - 6) + 2*pow(y, 2)*(2*y-3*pow(z, 2) + 3);
    float df_dz = 4*z*(3*pow(x, 2)*y - pow(y, 3) + 9*pow(z, 2) - 5);

    return Eigen::Vector3f(df_dx, df_dy, df_dz);
}


/**
 * WineGlassSurface class
 */

WineGlassSurface::WineGlassSurface(Eigen::Vector3f pos, Eigen::Vector3f range)
{
    bounding_box = AABB(pos - range / 2.0, pos + range / 2.0);
}

float WineGlassSurface::getValue(Eigen::Vector3f p)
{
    float x = p.x();
    float y = p.y();
    float z = p.z();

    /* Out of domain: Not-A-Number */
    if (z + 3.2 <= 0)
    {
        return INFINITY;
    }

    return x * x + y * y - pow(log(z + 3.2), 2) - 0.09;
}

Eigen::Vector3f WineGlassSurface::computeGradient(Eigen::Vector3f p)
{
    // TODO
    float x = p.x();
    float y = p.y();
    float z = p.z();

    float df_dx = 2 * x;
    float df_dy = 2 * y;
    float df_dz = -2 * log(z + 3.2) / (z + 3.2);

    return Eigen::Vector3f(df_dx, df_dy, df_dz);
}


/**
 * PorousSurface class
 */

PorousSurface::PorousSurface(Eigen::Vector3f pos, Eigen::Vector3f range)
{
    bounding_box = AABB(pos - range / 2.0, pos + range / 2.0);
}

float PorousSurface::getValue(Eigen::Vector3f p)
{
    float x = p.x();
    float y = p.y();
    float z = p.z();

    return -0.02 + pow(-0.88 + pow(y, 2), 2) * pow(2.92 * (-1 + x) * pow(x, 2) * (1 + x) + 1.7 * pow(y, 2), 2) + pow(-0.88 + pow(z, 2), 2) * pow(2.92 * (-1 + y) * pow(y, 2) * (1 + y) + 1.7 * pow(z, 2), 2) + pow(-0.88 + pow(x, 2), 2) * pow(1.7 * pow(x, 2) + 2.92 * (-1 + z) * pow(z, 2) * (1 + z), 2);
}

Eigen::Vector3f PorousSurface::computeGradient(Eigen::Vector3f p)
{
    // TODO
    float x = p.x();
    float y = p.y();
    float z = p.z();

    float df_dx = 6.8*x*pow((pow(x,2) - 0.88), 2)*(1.7*pow(x, 2) + 2.92*(z-1)*z*z*(z+1))
                  + 4*x*(x*x-0.88)*pow((1.7*x*x + 2.92*(z-1)*z*z*(z+1)), 2)
                  + 68.2112*(x*x*x-0.5*x)*pow(y*y-0.88, 2)*(pow(x, 4)-x*x+0.582192*y*y);

    float df_dy = 6.8*y*pow((pow(y,2) - 0.88), 2)*(1.7*pow(y, 2) + 2.92*(x-1)*x*x*(x+1))
                  + 4*y*(y*y-0.88)*pow((1.7*y*y + 2.92*(x-1)*x*x*(x+1)), 2)
                  + 68.2112*(y*y*y-0.5*y)*pow(z*z-0.88, 2)*(pow(y, 4)-y*y+0.582192*z*z);

    float df_dz = 6.8*z*pow((pow(z,2) - 0.88), 2)*(1.7*pow(z, 2) + 2.92*(y-1)*y*y*(y+1))
                  + 4*z*(z*z-0.88)*pow((1.7*z*z + 2.92*(y-1)*y*y*(y+1)), 2)
                  + 39.712*(z*z*z-0.5*z)*pow(x*x-0.88, 2)*(1.71765*pow(z, 4)-1.71765*z*z+x*x);

    return Eigen::Vector3f(df_dx, df_dy, df_dz);
}

/**
 * GenusTwoSurface class
 */

TwoObeject::TwoObeject(Eigen::Vector3f pos, Eigen::Vector3f range)
{
    bounding_box = AABB(pos - range / 2.0, pos + range / 2.0);
}

float TwoObeject::getValue(Eigen::Vector3f p)
{
    float x1 = p.x() + 2;
    float x2 = p.x() - 2;

    float y = p.y();
    float z = p.z();

    if (z + 3.2 <= 0)
    {
        return INFINITY;
    }

    float obj1 = 2 * y * (y * y - 3 * x1 * x1) * (1 - z * z) + pow(x1 * x1 + y * y, 2) - (9 * z * z - 1) * (1 - z * z);
    float obj2 = x2 * x2 + y * y - pow(log(z + 3.2), 2) - 0.09;
    if (p.x() < 0)
        return obj1;
    else
        return obj2;
}

Eigen::Vector3f TwoObeject::computeGradient(Eigen::Vector3f p)
{
    // TODO
    float x1 = p.x() + 2;
    float x2 = p.x() - 2;

    float y = p.y();
    float z = p.z();

    float df_dx1 = 4.0f*(pow(x1, 3) + x1*y*(y+3*pow(z, 2)-3));
    float df_dy1 = pow(x1, 2) * (4*y + 6*pow(z, 2) - 6) + 2*pow(y, 2)*(2*y-3*pow(z, 2) + 3);
    float df_dz1 = 4*z*(3*pow(x1, 2)*y - pow(y, 3) + 9*pow(z, 2) - 5);

    float df_dx2 = 2 * x2;
    float df_dy2 = 2 * y;
    float df_dz2 = -2 * log(z + 3.2) / (z + 3.2);

    if (p.x() < 0)
        return Eigen::Vector3f(df_dx1, df_dy1, df_dz1);
    else
        return Eigen::Vector3f(df_dx2, df_dy2, df_dz2);
}
