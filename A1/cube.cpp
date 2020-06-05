#include "cube.hpp"

#include <glm/glm.hpp>

using namespace glm;

//----------------------------------------------------------------------------------------
// Cube Constructor
Cube::Cube(float n)
	: verts(nullptr), indices(nullptr), numVerts(8), numIndices(3*2*8), n(n)
{
	vec3 cubeVerts[] = {
		vec3(0,0,0),   // 0
		vec3(0,0,n),   // 1
		vec3(0,n,0),   // 2
		vec3(0,n,n),   // 3
		vec3(n,0,0),   // 4
		vec3(n,0,n),   // 5
		vec3(n,n,0),   // 6
		vec3(n,n,n)    // 7
	};

	unsigned int cubeIndices[] = {
		// Front face
		0, 4, 6,
		6, 2, 0,

		// Right face
		4, 5, 7,
		7, 6, 4,

		// Back face
		5, 1, 3,
		3, 7, 5,

		// Left face
		1, 0, 2,
		2, 3, 1,

		// Top face
		2, 6, 7,
		7, 3, 2,

		// Bottom face
		0, 4, 5,
		5, 1, 0
	};

	verts = new vec3[numVerts];
	indices = new unsigned int[numIndices];

	for(int i = 0; i < numVerts; ++i)
		verts[i] = cubeVerts[i];
	
	for(int i = 0; i < numIndices; ++i)
		indices[i] = cubeIndices[i];
}

//----------------------------------------------------------------------------------------
// Cube Destructor
Cube::~Cube()
{
	if (verts) delete [] verts;
	if (indices) delete [] indices;
}
