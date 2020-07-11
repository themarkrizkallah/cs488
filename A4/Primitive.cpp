// Spring 2020

#include "Primitive.hpp"
#include "Common.hpp"
#include "polyroots.hpp"

#include <iostream>
#include <glm/glm.hpp>

using namespace std;
using namespace glm;

// ------------------------------------------------------------
// Primitive
Primitive::~Primitive()
{}

HitRecord Primitive::hit(const Ray &r) const 
{
    return HitRecord();
}

// ------------------------------------------------------------
// Sphere
Sphere::~Sphere()
{}

// ------------------------------------------------------------
// Cube
Cube::~Cube()
{}


// ------------------------------------------------------------
// Non-hierarchal Sphere
NonhierSphere::NonhierSphere(const glm::vec3& pos, double radius)
    : m_pos(pos), m_radius(radius)
{}

NonhierSphere::~NonhierSphere()
{}

/* Compute Ray-Sphere Intersection, store hit record in rec
 * Fundamentals of Computer Graphics 4.4.1
 */
HitRecord NonhierSphere::hit(const Ray &r) const
{
    HitRecord rec;

    const vec3 e(r.origin.x, r.origin.y, r.origin.z);
    const vec3 d(r.direction());
    const vec3 centreDir = e - m_pos;

    // Quadratic formula parameters
    const double A = glm::dot(d, d);
    const double B = 2.0 * glm::dot(d, centreDir);
    const double C = glm::dot(centreDir, centreDir) - (m_radius * m_radius);

    double roots[2];
    auto numRoots = quadraticRoots(A, B, C, roots);

    auto minRoot = std::min(roots[0], roots[1]);
    auto maxRoot = std::max(roots[0], roots[1]);

    // No roots or max root insignifcant, no hit
    if(numRoots <= 0 || maxRoot < EPSILON)
		return rec;

    rec.hit = true;
    rec.t = (minRoot >= EPSILON)? minRoot : maxRoot;
    rec.n = r.pointAt(rec.t) - vec4(m_pos, 1);
    rec.point = r.pointAt(rec.t);

    return rec;
}

// ------------------------------------------------------------
// Non-hierarchal Box
NonhierBox::NonhierBox(const glm::vec3& pos, double size)
    : m_pos(pos), m_size(size)
{}

NonhierBox::~NonhierBox()
{}

// TODO
HitRecord NonhierBox::hit(const Ray &r) const 
{
    return false;
}