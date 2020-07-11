#include "Ray.hpp"

#include <memory>
#include <iostream>
#include <glm/glm.hpp>

// #define OLD_RAY

using namespace glm;

#ifdef OLD_RAY
// Default Constructor
Ray::Ray(const vec4 &origin, const vec4 &dest)
    : origin(origin), dest(dest)
{}

// Copy Constructor
Ray::Ray(const Ray &other)
    : origin(other.origin), dest(other.dest)
{}

// Move Constructor
Ray::Ray(Ray &&other)
    : origin(std::move(other.origin)), dest(std::move(other.dest))
{}

// Compute ray direction, dest - origin
vec4 Ray::direction() const{
    return glm::normalize(dest - origin);
}

// Compute point on the ray using p(t) = e + td 
vec4 Ray::pointAt(float t) const
{
    vec4 point = origin + t * direction();
    point.w = 1.0f;
    return point;
}

Ray operator*(const mat4 &M, const Ray& r)
{
    vec4 transformedOrigin = M * r.origin; 
    transformedOrigin.w = 1.0f;

    vec4 transformedDest = M * r.dest; 
    transformedDest.w = 1.0f;

    return Ray(transformedOrigin, transformedDest);
}
#else
// Default Constructor
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

// Compute point on the ray using p(t) = e + td 
vec4 Ray::pointAt(float t) const
{
    return origin + t*direction;
}

Ray operator*(const mat4 &M, const Ray& r)
{
    return Ray(M * r.origin, M * r.direction);
}

#endif

// Default Constructor
HitRecord::HitRecord(bool hit, double t, const vec4 &n, const vec4 &point, Material *mat)
    : hit(hit), t(t), n(n), point(point), mat(mat)
{}

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