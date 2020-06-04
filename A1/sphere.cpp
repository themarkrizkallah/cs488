#include "sphere.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

static const float PI = glm::pi<float>();

//----------------------------------------------------------------------------------------
// Sphere Constructor
Sphere::Sphere(float r, float longs, float lats)
	: verts(nullptr), indices(nullptr), r(r), longs(longs), lats(lats), numVerts(longs*lats*5), numIndices(longs*lats*6)
{}

//----------------------------------------------------------------------------------------
// Cube Destructor
Sphere::~Sphere()
{
	if (verts) delete [] verts;
	if (indices) delete [] indices;
}

//----------------------------------------------------------------------------------------
// Cube Destructor
void Sphere::computeVerts()
{
	verts = new vec3[numVerts];
	indices = new unsigned int[numIndices];

	int i = 0, j = 0;
	float x, y, z;
	float phi, theta;

	for (float lng = 0; lng < longs; ++lng) {
		for (float lat = lats/2.0f; lat > -(lats/2.0f); --lat) {
			/* Top triangle*/

			// Vertex 1
			phi = lat * PI/lats;
			theta = lng * PI/(longs/2.0f);

			x = r * glm::cos(phi) * glm::cos(theta);
			y = r * glm::sin(phi);
			z = r * glm::cos(phi) * glm::sin(theta);

			indices[j++] = i;
			verts[i++] = vec3(x, y, z);

			// Vertex 2
			phi = (lat - 1) * PI/lats;
			theta = lng * PI/(longs/2.0f);

			x = r * glm::cos(phi) * glm::cos(theta);
			y = r * glm::sin(phi);
			z = r * glm::cos(phi) * glm::sin(theta);

			indices[j++] = i;
			verts[i++] = vec3(x, y, z);

			// Vertex 3
			phi = lat * PI/lats;
			theta = (lng + 1) * PI/(longs/2.0f);

			x = r * glm::cos(phi) * glm::cos(theta);
			y = r * glm::sin(phi);
			z = r * glm::cos(phi) * glm::sin(theta);

			indices[j++] = i;
			verts[i++] = vec3(x, y, z);

			/* Bottom triangle*/

			// Vertex 1
			phi = (lat-1) * PI/lats;
			theta = lng * PI/(longs/2.0f);

			x = r * glm::cos(phi) * glm::cos(theta);
			y = r * glm::sin(phi);
			z = r * glm::cos(phi) * glm::sin(theta);

			indices[j++] = i;
			verts[i++] = vec3(x, y, z);

			// Vertex 2 (identical to Triangle 1 vertex 3)
			indices[j++] = i-2;

			// Vertex 3
			phi = (lat-1) * PI/lats;
			theta = (lng+1) * PI/(longs/2.0f);

			x = r * glm::cos(phi) * glm::cos(theta);
			y = r * glm::sin(phi);
			z = r * glm::cos(phi) * glm::sin(theta);

			indices[j++] = i;
			verts[i++] = vec3(x, y, z);
		}
	}
}
