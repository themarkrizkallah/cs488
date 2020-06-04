#pragma once
#ifndef SPHERE_H
#define SPHERE_H

#include <glm/glm.hpp>

struct Sphere {
	glm::vec3 *verts;
	unsigned int *indices;

	float r;     // Radius
	float longs; // Number of longitude lines
	float lats;  // Number of latitude lines

	size_t numVerts;   // Number of vertices needed to render sphere
	size_t numIndices; // Number of indices needed to map vertices in the ibo

	Sphere(float r, float longs = 360.0f, float lats = 180.0f);
	~Sphere();

	void computeVerts();
};
#endif