// Spring 2020

#pragma once

#include "Ray.hpp"
#include "Epsilon.hpp"
#include <utility>
#include <glm/glm.hpp>

// ------------------------------------------------------------
// Primitive
class Primitive {
public:
  virtual ~Primitive();
  virtual HitRecord hit(const Ray &r, double t0, double t1) const;
};

// ------------------------------------------------------------
// Non-hierarchal Sphere
class NonhierSphere : public Primitive {
public:
  NonhierSphere(const glm::vec3& pos, double radius);
  virtual ~NonhierSphere();

  virtual HitRecord hit(const Ray &r, double t0, double t1) const override;

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

  virtual HitRecord hit(const Ray &r, double t0, double t1) const override;

private:
  glm::vec3 m_pos;
  double m_size;
};

// ------------------------------------------------------------
// Sphere
class Sphere : public Primitive {
public:
  Sphere();
  virtual ~Sphere();

  virtual HitRecord hit(const Ray &r, double t0, double t1) const override;

private:
  NonhierSphere *m_sphere;
};

// ------------------------------------------------------------
// Cube
class Cube : public Primitive {
public:
  Cube();
  virtual ~Cube();

  virtual HitRecord hit(const Ray &r, double t0, double t1) const override;

private:
  NonhierBox *m_box;
};
