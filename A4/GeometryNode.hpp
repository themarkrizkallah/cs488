// Spring 2020

#pragma once

#include "SceneNode.hpp"
#include "Primitive.hpp"
#include "Material.hpp"
#include "Ray.hpp"

// Used as a "Surface" (Fundamentals of Computer Graphics 4.6.1)
class GeometryNode : public SceneNode {
public:
	GeometryNode( const std::string & name, Primitive *prim, 
		Material *mat = nullptr );

	void setMaterial( Material *material );

	Material *m_material;
	Primitive *m_primitive;

    // (Fundamentals of Computer Graphics 4.6.1)
	// HitRecord hit(const Ray &r, const glm::mat4 &worldToModel) const override;
};
