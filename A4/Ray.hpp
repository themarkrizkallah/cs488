#pragma once

#include "Material.hpp"
#include <glm/glm.hpp>

#include <string>
#include <limits>

struct Ray {
    Ray(const glm::vec4 &origin = glm::vec4(0,0,0,1), const glm::vec4 &direction = glm::vec4(0));
    Ray(const Ray &other);
    Ray(Ray &&other);

    Ray &operator=(const Ray &other);
    Ray &operator=(Ray &&other);

    glm::vec4 origin;    // Ray origin
    glm::vec4 direction; // Ray direction

    // glm::vec4 direction() const;
    glm::vec4 pointAt(float t) const;
};
Ray operator*(const glm::mat4 &M, const Ray& r);

struct HitRecord {
    HitRecord(
        bool hit = false, 
        double t = std::numeric_limits<double>::infinity(), 
        const glm::vec4 &n = glm::vec4(0), 
        const glm::vec4 &point = glm::vec4(0,0,0,1), 
        Material *mat = nullptr,
        std::string const *name = nullptr 
    );

    HitRecord &operator=(const HitRecord &other);

    bool hit;          // True if surface was hit
    double t;          // Ray position where intersection occured
    glm::vec4 n;       // Intersection normal
    glm::vec4 point;   // Intersection point
    Material *mat;     // Material of hit object
    std::string const *name; // Hit node's name

    explicit operator bool() const;

    bool operator<(const HitRecord &other) const;
    bool operator<=(const HitRecord &other) const;
    bool operator>(const HitRecord &other) const;
    bool operator>=(const HitRecord &other) const;
};
