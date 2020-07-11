#pragma once

#include "Material.hpp"
#include <glm/glm.hpp>

#define OLD_RAY

#include <limits>

#ifdef OLD_RAY
struct Ray {
    Ray(const glm::vec4 &origin = glm::vec4(0,0,0,1), const glm::vec4 &dest = glm::vec4(0,0,0,1));
    Ray(const Ray &other);
    Ray(Ray &&other);

    glm::vec4 origin; // Ray origin (e) (point)
    glm::vec4 dest;   // Ray "destination" (point)

    glm::vec4 direction() const;
    glm::vec4 pointAt(float t) const;
};
#else
struct Ray {
    Ray(const glm::vec4 &origin = glm::vec4(), const glm::vec4 &direction = glm::vec4());
    Ray(const Ray &other);
    Ray(Ray &&other);

    glm::vec4 origin;    // Ray origin (e) (point)
    glm::vec4 direction; // Ray direction (d) (vector)

    glm::vec4 pointAt(float t) const;
};
#endif
Ray operator*(const glm::mat4 &M, const Ray& r);

struct HitRecord {
    HitRecord(
        bool hit = false, 
        double t = std::numeric_limits<double>::infinity(), 
        const glm::vec4 &n = glm::vec4(), 
        const glm::vec4 &point = glm::vec4(0,0,0,1), 
        Material *mat = nullptr
    );

    bool hit;        // True if surface was hit
    double t;        // Ray position where intersection occured
    glm::vec4 n;     // Intersection normal
    glm::vec4 point; // Intersection point
    Material *mat;   // Material of hit object

    bool operator<(const HitRecord &other) const;
    bool operator<=(const HitRecord &other) const;
    bool operator>(const HitRecord &other) const;
    bool operator>=(const HitRecord &other) const;
};