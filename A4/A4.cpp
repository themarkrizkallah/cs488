// Spring 2020

#include "Material.hpp"
#include "PhongMaterial.hpp"
#include "Epsilon.hpp"
#include "A4.hpp"

#include <vector>
#include <utility>
#include <future>
#include <thread>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

// Multithreading flag
#define ENABLE_MULTITHREADING

using namespace std;
using namespace glm;

static const uint NUM_CORES = thread::hardware_concurrency();

// Generate the DCS to WCS matrix (Course Notes 20.1 SI)
mat4 generateDCStoWorldMat(
	const pair<size_t, size_t> &pixelDim,
	const vec3 &eye,
	const vec3 &view,
	const vec3 &up,
	const double fovy
)
{
	// Pixel dimensions, (n_x, n_y)
	const auto n_x = (double) pixelDim.first;
	const auto n_y = (double) pixelDim.second;

	// "Focal" distance
	const double d = 1.0;

	// Aspect ratio
	const double height = 2 * d * glm::tan(glm::radians(fovy/2));
	const double width = (n_x / n_y) * height;

	// Step 1: Create translation matrix T1
	mat4 T1 = glm::translate(vec3(-n_x/2.0, -n_y/2.0, d));

	// Step 2: Preserve the aspect ratio and change x-axis direction using S2
	// Note: Pixel y values grow towards the bottom, reverse y
	mat4 S2 = glm::scale(vec3(-height/n_y, -width/n_x, 1));

	// Step 3: Rotate using R3
	vec3 w = glm::normalize(view);
	vec3 u = glm::normalize(glm::cross(up, w));
	vec3 v = glm::cross(w, u);

	mat4 R3(
		vec4(u, 0),
		vec4(v, 0),
		vec4(w, 0),
		vec4(0, 0, 0, 1)
	);

	// Step 4: Translate by Lookfrom (eye)
	mat4 T4 = glm::translate(eye);

	return T4 * R3 * S2 * T1;
}


vec3 rayColour(
	SceneNode *node,
	const Ray &r,
	const int hitCount,
	const vec3 &ambient,
	const list<Light *> &lights
)
{
	HitRecord rec = node->hit(r, EPSILON, INF_DOUBLE);

	// Hit, compute shadow rays
	if(rec.hit)
		return directColour(node, r, rec, ambient, lights);	

	// No hit, use background colour
	else
		return BackgroundColour;
}

vec3 directColour(
	SceneNode *node,
	const Ray &primRay,
	const HitRecord &primRec,
	const vec3 &ambient,
	const list<Light *> &lights
)
{
	// Material
	const auto mat = static_cast<PhongMaterial *>(primRec.mat);

	// Diffuse, specular, and shininess components
	const auto &kd = mat->diffuse();
	const auto &ks = mat->specular();
	const auto &ke = mat->shininess();

	// Ambient component
	auto col = kd * ambient;

	const vec4 n = glm::normalize(primRec.n);          // Intersection point normal (normalized)
	const vec4 p = primRec.point + CORRECTION * n;     // Intersection point (corrected)
	const vec4 v = glm::normalize(primRay.origin - p); // Intersection to eye point vector

	// Compute shadow rays
	for(const auto light : lights){
		const Ray shadowRay(p, vec4(light->position, 1));

		// Shade pixel if shadow ray isn't obstructed
		HitRecord rec = node->hit(shadowRay, EPSILON, INF_DOUBLE);
		if(!rec.hit){
			// Blinn-Phong Shading
			const vec3 &I = light->colour;

			vec4 l = glm::normalize(shadowRay.direction()); // Light direction
			vec4 h = glm::normalize(v + l); // Halfway vector

			// Diffuse component
			col += kd * I * std::max(0.0f, glm::dot(n, l));

			// Specular component
			col += ks * I * std::pow(std::max(0.0f, glm::dot(n, h)), ke);
		}
	}

	return col;
}

static void renderChunk(
	const uint n_y,
	const uint xStart, const uint xEnd,

	Image &image,

	SceneNode *root,

	const mat4 &dcsToWorld,
	const vec4 &eye,

	const vec3 & ambient,
	const list<Light *> & lights

)
{
	for(uint x = xStart; x < xEnd; ++x) {
		for(uint y = 0; y < n_y; ++y){
			const vec4 p_world = dcsToWorld * vec4(x, y, 0, 1);
			const Ray ray(eye, p_world);

			// Compute pixel colour
			const auto col = rayColour(root, ray, 0, ambient, lights);

			// Red: 
			image(x, y, Cone::R) = col[Cone::R];
			// Green: 
			image(x, y, Cone::G) = col[Cone::G];
			// Blue: 
			image(x, y, Cone::B) = col[Cone::B];
		}
	}
}

void A4_Render(
		// What to render  
		SceneNode * root,

		// Image to write to, set to a given width and height  
		Image & image,

		// Viewing parameters  
		const vec3 & eye,
		const vec3 & view,
		const vec3 & up,
		double fovy,

		// Lighting parameters  
		const vec3 & ambient,
		const list<Light *> & lights
) {
	// Fill in raytracing code here...  
	cout << "Calling A4_Render(\n" <<
		  "\t" << *root <<
          "\t" << "Image(width:" << image.width() << ", height:" << image.height() << ")\n"
          "\t" << "eye:  " << glm::to_string(eye) << endl <<
		  "\t" << "view: " << glm::to_string(view) << endl <<
		  "\t" << "up:   " << glm::to_string(up) << endl <<
		  "\t" << "fovy: " << fovy << endl <<
		  "\t" << "n_x: " << image.width() << endl << 
		  "\t" << "n_y: " << image.height() << endl <<
          "\t" << "ambient: " << glm::to_string(ambient) << endl <<
		  "\t" << "lights{" << endl;


	for(const Light * light : lights)
		std::cout << "\t\t" <<  *light << endl;

	cout << "\t}" << endl;
	cout <<")" << endl;

	// Image dimensions
	const size_t n_x = image.width();
	const size_t n_y = image.height();
	const auto pixelDim = std::make_pair(n_x, n_y);

	// DCS to WCS matrix
	const mat4 dcsToWorld = generateDCStoWorldMat(pixelDim, eye, view, up, fovy);

	// Eye 4D point
	const vec4 eye4D(eye, 1);

	/* Ray Trace image */

#ifdef ENABLE_MULTITHREADING
	const uint numWorkers = NUM_CORES;
	const uint chunkSize = std::ceil(double(n_x) / double(numWorkers));

	cout << endl << "Multithreading enabled: " << endl;
		cout << "\t " << numWorkers << "   workers" << endl;
		cout << "\t~" << chunkSize << " columns/worker" << endl;

	std::vector<std::future<void>> workers;
	workers.reserve(numWorkers);

	uint xStart = 0;
	uint xEnd = chunkSize;

	// Chunk the image and launch workers
	for(uint chunk = 0; chunk < numWorkers; ++chunk) {
		xStart = chunk * chunkSize;
		xEnd = chunk < numWorkers - 1 ? xStart + chunkSize : n_x;

		workers.push_back(std::async(
			// Type
			std::launch::async,

			// Function
			renderChunk,

			// Parameters
			n_y,
			xStart, xEnd,
			std::ref(image),
			root,
			std::ref(dcsToWorld),
			std::ref(eye4D),
			std::ref(ambient),
			std::ref(lights)
		));
	}

	// Wait for workers to finish
	for(const auto &worker : workers)
		worker.wait();

#else
	for (uint x = 0; x < n_x; ++x) {
		for (uint y = 0; y < n_y; ++y) {
			const vec4 p_world = dcsToWorld * vec4(x, y, 0, 1);
			const Ray ray(eye4D, p_world);

			// Compute pixel colour
			const auto col = rayColour(root, ray, 0, ambient, lights);

			// Red: 
			image(x, y, Cone::R) = col[Cone::R];
			// Green: 
			image(x, y, Cone::G) = col[Cone::G];
			// Blue: 
			image(x, y, Cone::B) = col[Cone::B];
		}
	}
#endif
}
