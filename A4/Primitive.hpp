// Spring 2020

#pragma once

#include "Ray.hpp"

#include <utility>
#include <glm/glm.hpp>

// ------------------------------------------------------------
// Primitive
class Primitive {
public:
  virtual ~Primitive();
  virtual HitRecord hit(const Ray &r) const;
};

// ------------------------------------------------------------
// Sphere
class Sphere : public Primitive {
public:
  virtual ~Sphere();
};

// ------------------------------------------------------------
// Cube
class Cube : public Primitive {
public:
  virtual ~Cube();
};

// ------------------------------------------------------------
// Non-hierarchal Sphere
class NonhierSphere : public Primitive {
public:
  NonhierSphere(const glm::vec3& pos, double radius);
  virtual ~NonhierSphere();

  // Fundamentals of Computer Graphics 4.4.1
  virtual HitRecord hit(const Ray &r) const override;

private:
  glm::vec3 m_pos;
  double m_radius;
};

// ------------------------------------------------------------
// Non-hierarchal Box
class NonhierBox : public Primitive {
public:
  NonhierBox(const glm::vec3& pos, double size);  
  virtual ~NonhierBox();

  virtual HitRecord hit(const Ray &r) const;

private:
  glm::vec3 m_pos;
  double m_size;
};
