// Spring 2020

#pragma once

#include <glm/glm.hpp>
#include <limits>

#include "SceneNode.hpp"
#include "Light.hpp"
#include "Ray.hpp"
#include "Colour.hpp"
#include "Image.hpp"

const int MAX_HITS = 100;

const Colour BackgroundColour(0.529, 0.808, 0.922);

glm::mat4 generateDCStoWorldMat(
	// Pixel dimensions, (n_x, n_y)
	const std::pair<size_t, size_t> &pixelDim,

	// View parameters
	const glm::vec3 &eye,
	const glm::vec3 &view,
	const glm::vec3 &up,
	const double fovy
);

Colour rayColour(
	SceneNode *node, 
	const Ray &r, 
	const int hitCount,
	const glm::vec3 &ambient,
	const std::list<Light *> &lights
);

Colour directColour(
	SceneNode *node,
	const Ray &primRay,
	const HitRecord &primRec,
	const glm::vec3 &ambient,
	const std::list<Light *> &lights
);

void A4_Render(
		// What to render
		SceneNode * root,

		// Image to write to, set to a given width and height
		Image & image,

		// Viewing parameters
		const glm::vec3 & eye,
		const glm::vec3 & view,
		const glm::vec3 & up,
		double fovy,

		// Lighting parameters
		const glm::vec3 & ambient,
		const std::list<Light *> & lights
);

// glm::vec2 computeUV(
// 	const std::pair<size_t, size_t> &pixelDim,
// 	const std::pair<double, double> &dim,
// 	const glm::vec2 &pixelPos
// );