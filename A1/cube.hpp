#pragma once
#ifndef CUBE_H
#define CUBE_H

#include <glm/glm.hpp>

// Cube struct
struct Cube {
	glm::vec3 *verts;
	unsigned int *indices;

	const size_t numVerts;   // Number of vertices required to render cube
	const size_t numIndices; // Number of indices required to map vertices in the ibo
	const float n;

	Cube(float n);
	~Cube();
};
#endif