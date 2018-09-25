#ifndef __CVK_SPHERE_H
#define __CVK_SPHERE_H

#include "CVK_Geometry.h"

class Sphere : public Geometry
{
public:
    Sphere();
    Sphere( float radius);
    Sphere( glm::vec3 center, float radius);
    Sphere( float radius, int resolution);
    Sphere( glm::vec3 center, float radius, int resolution);
    ~Sphere();

    glm::vec3 *get_center();
    float get_radius();

private:
    void create( glm::vec3 center, float radius, int resolution);

    glm::vec3 m_Center;
    float m_radius;
    int m_resolution;
};



#endif /* __CVK_SPHERE_H */