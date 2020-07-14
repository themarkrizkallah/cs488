// Spring 2020

#pragma once

#include <glm/glm.hpp>
#include <limits>

#include "SceneNode.hpp"
#include "Light.hpp"
#include "Ray.hpp"
#include "Image.hpp"

const int MAX_HITS = 100;


const glm::vec3 ZenithColour(0.0f, 0.0f, 0.35f);
const glm::vec3 DuskColour(0.902f, 0.514f, 0.071f);

enum Cone {
	R,
	G,
	B
};

glm::mat4 generateDCStoWorldMat(
	// Pixel dimensions, (n_x, n_y)
	const std::pair<size_t, size_t> &pixelDim,

	// View parameters
	const glm::vec3 &eye,
	const glm::vec3 &view,
	const glm::vec3 &up,
	const double fovy
);

glm::vec3 rayColour(
	SceneNode *node, 
	const Ray &r, 
	const int hitCount,
	const glm::vec3 &ambient,
	const std::list<Light *> &lights
);

glm::vec3 directColour(
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
