// Spring 2020

#pragma once

#include "Options.hpp"
#include "Primitive.hpp"

#include <vector>
#include <iosfwd>
#include <string>
#include <memory>

#include <glm/glm.hpp>

struct Triangle
{
	size_t v1;
	size_t v2;
	size_t v3;

	Triangle(size_t pv1, size_t pv2, size_t pv3)
		: v1(pv1),
		  v2(pv2),
		  v3(pv3)
	{}

	HitRecord hit(const Ray &r, double t0, double t1, const glm::vec3 *verts) const;
};

// A polygonal mesh.
class Mesh : public Primitive {
public:
	Mesh(const std::string& fname);

	virtual HitRecord hit(const Ray &r, double t0, double t1) const override;
  
private:
	std::vector<glm::vec3> m_vertices;
	std::vector<Triangle> m_faces;

#ifdef ENABLE_BOUNDING_VOLUMES
	glm::vec3 m_boundingMin;
	glm::vec3 m_boundingMax;
	std::unique_ptr<Primitive> m_bv; // bounding volume

	Primitive *boundingVolume(BoundingVolume volType = BoundingVolume::BoundingBox) const;
#endif

    friend std::ostream& operator<<(std::ostream& out, const Mesh& mesh);
};
