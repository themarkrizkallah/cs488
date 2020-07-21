// Spring 2020

#include "Primitive.hpp"
#include "Epsilon.hpp"
#include "polyroots.hpp"

#include <iostream>
#include <glm/glm.hpp>

using namespace std;
using namespace glm;

// ------------------------------------------------------------
// Primitive
Primitive::~Primitive()
{}

HitRecord Primitive::hit(const Ray &r, double t0, double t1) const 
{
    return HitRecord();
}

// ------------------------------------------------------------
// Non-hierarchal Sphere
NonhierSphere::NonhierSphere(const glm::vec3& pos, double radius)
    : m_pos(pos), m_radius(radius)
{}

NonhierSphere::~NonhierSphere()
{}

// Compute Ray-Sphere Intersection, store hit record in rec
HitRecord NonhierSphere::hit(const Ray &r, double t0, double t1) const
{
    HitRecord rec;
	double t;

    const vec3 e(r.origin.x, r.origin.y, r.origin.z);
    const vec3 d(r.direction);
    const vec3 centreDir = e - m_pos;

    // Quadratic formula parameters
    const double A = glm::dot(d, d);
    const double B = 2.0 * glm::dot(d, centreDir);
    const double C = glm::dot(centreDir, centreDir) - (m_radius * m_radius);

    double roots[2];
    auto numRoots = quadraticRoots(A, B, C, roots);

    auto minRoot = std::min(roots[0], roots[1]);
    auto maxRoot = std::max(roots[0], roots[1]);

	t = minRoot > t0 ? minRoot : maxRoot;

	// Check if solution exists and is in (t0, t1)
	if(numRoots > 0 && t > t0 && t < t1){
		rec.hit = true;
		rec.t = t;
		rec.point = r.pointAt(rec.t);
		rec.n = rec.point - vec4(m_pos, 1);
	}

    return rec;
}

// ------------------------------------------------------------
// Non-hierarchal Box
NonhierBox::NonhierBox(const glm::vec3& pos, double size)
    : m_pos(pos), m_size(size)
{}

NonhierBox::NonhierBox(const glm::vec3& pos, glm::vec3 size)
    : m_pos(pos), m_size(size)
{}

NonhierBox::~NonhierBox()
{}

HitRecord NonhierBox::hit(const Ray &r, double t0, double t1) const 
{
    HitRecord rec;
	double t;
	vec4 normal;

	// Inverse ray direction here and then multiply moving forward
    const vec4 rayDirection = 1.0f / r.direction;

	const auto closeCorner = m_pos;
	const auto farCorner = m_pos + m_size;

	// Solve for t using the x-interval
	double tMin = (closeCorner.x - r.origin.x) * rayDirection.x;
	double tMax = (farCorner.x - r.origin.x) * rayDirection.x;

    // Left and right face normals
	vec4 closeNormal(-1, 0, 0, 0);
	vec4 farNormal(1, 0, 0, 0);

    // Reverse t and normals if box is reversed
	if(tMax < tMin){
		std::swap(tMin, tMax);
		std::swap(closeNormal, farNormal);
	}

	// Solve for t using the y-interval
	double y_tMin = (closeCorner.y - r.origin.y) * rayDirection.y;
	double y_tMax = (farCorner.y - r.origin.y) * rayDirection.y;

    // Down and up face normals
	vec4 y_closeNormal(0, -1, 0, 0);
	vec4 y_farNormal(0, 1, 0, 0);

    // Reverse t and normals if box is reversed
	if(y_tMax < y_tMin){
		std::swap(y_tMin, y_tMax);
		std::swap(y_closeNormal, y_farNormal);
	}

    // No solution if the x and y intervals do not overlapt
	if(y_tMax < tMin or tMax < y_tMin)
		return rec;

    // Properly set the lower bound
    // Lower bound = max(tMin, y_tMin)
	if(std::isnan(tMin) || tMin < y_tMin){
		tMin = y_tMin;
		closeNormal = y_closeNormal;
	}

    // Properly set the upper bound
    // Upper bound = min(tMax, y_tMax)
	if(std::isnan(tMax) || y_tMax < tMax){
		tMax = y_tMax;
		farNormal = y_farNormal;
	}

	// Solve for t using the Z-interval
	double z_tMin = (closeCorner.z - r.origin.z) * rayDirection.z;
	double z_tMax = (farCorner.z -  r.origin.z) * rayDirection.z;

    // Back and front face normals
	vec4 z_closeNormal(0, 0, -1, 0);
	vec4 z_farNormal(0, 0, 1, 0);

    // Reverse t and normals if box is reversed
	if(z_tMax < z_tMin){
		std::swap(z_tMin, z_tMax);
		std::swap(z_closeNormal, z_farNormal);
	}

    // No solution if the current interval and z-interval do not overlapt
	if(z_tMax < tMin or tMax < z_tMin)
		return rec;

    // Properly set the lower bound
    // Lower bound = max(tMin, z_tMin)
	if(std::isnan(tMin) || tMin < z_tMin){
		tMin = z_tMin;
		closeNormal = z_closeNormal;
	}

    // Properly set the upper bound
    // Upper bound = min(tMax, z_tMax)
	if(std::isnan(tMax) || z_tMax < tMax){
		tMax = z_tMax;
		farNormal = z_farNormal;
	}

    // Check that the intersection is within range
	if(tMax < EPSILON)
		return rec;

    // Check that the eye is not in the middle of the box
    // If it is, use tMax
	if(tMin < EPSILON){
		t = tMax;
		normal = farNormal;
	} else {
		t = tMin;
		normal = closeNormal;
	}

	if(t > t0 && t < t1){
		rec.hit = true;
		rec.t = t;
		rec.n = normal;
		rec.point = r.pointAt(rec.t);
	}

    return rec;
}


// ------------------------------------------------------------
// Sphere
Sphere::Sphere()
	: m_sphere()
{}

Sphere::~Sphere()
{}

HitRecord Sphere::hit(const Ray &r, double t0, double t1) const
{
    return m_sphere.hit(r, t0, t1);
}

// ------------------------------------------------------------
// Cube
Cube::Cube(): m_box()
{}

Cube::~Cube()
{}

HitRecord Cube::hit(const Ray &r, double t0, double t1) const
{
    return m_box.hit(r, t0, t1);
}