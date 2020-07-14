#include "Ray.hpp"
#include "Epsilon.hpp"

#include <memory>
#include <iostream>
#include <glm/glm.hpp>

using namespace glm;

// ------------------------------------------------------------
// Ray
Ray::Ray(const vec4 &origin, const vec4 &direction)
    : origin(origin), direction(direction)
{}

// Copy Constructor
Ray::Ray(const Ray &other)
    : origin(other.origin), direction(other.direction)
{}

// Move Constructor
Ray::Ray(Ray &&other)
    : origin(std::move(other.origin)), direction(std::move(other.direction))
{}

// Copy assignment
Ray &Ray::operator=(const Ray &other)
{
    if(this != &other){
        origin = other.origin;
        direction = other.direction;
    }

    return *this;
}

// Move assignment
Ray &Ray::operator=(Ray &&other)
{
    origin = std::move(other.origin);
    direction = std::move(other.direction);

    return *this;
}

// Compute point on the ray using p(t) = e + td 
vec4 Ray::pointAt(float t) const
{
    return origin + t * direction;
}

Ray operator*(const mat4 &M, const Ray& r)
{
    return Ray(M * r.origin, M * r.direction);
}


// ------------------------------------------------------------
// HitRecord
HitRecord::HitRecord(bool hit, double t, const vec4 &n, const vec4 &point, Material *mat)
    : hit(hit), t(t), n(n), point(point), mat(mat)
{}

// Copy assignment
HitRecord &HitRecord::operator=(const HitRecord &other)
{
    if(this != &other){
        hit = other.hit;
        t = other.t;
        n = other.n;
        point = other.point;
        mat = other.mat;
    }

    return *this;
}

HitRecord::operator bool() const
{
    return hit;
}

// Comparison operators
bool HitRecord::operator<(const HitRecord &other) const
{
    return t < other.t;
}

bool HitRecord::operator<=(const HitRecord &other) const
{
    return t <= other.t;
}

bool HitRecord::operator>(const HitRecord &other) const
{
    return t > other.t;
}

bool HitRecord::operator>=(const HitRecord &other) const
{
    return t >= other.t;
}
