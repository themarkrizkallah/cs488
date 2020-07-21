// Spring 2020

#include "Options.hpp"
#include "Epsilon.hpp"
#include "Material.hpp"
#include "PhongMaterial.hpp"
#include "Timer.hpp"
#include "A4.hpp"

#include <iostream>
#include <iomanip>
#include <chrono>
#include <string>
#include <vector>
#include <utility>
#include <future>
#include <thread>
#include <mutex>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

using namespace std;
using namespace glm;


#ifdef ENABLE_MULTITHREADING
static mutex ioMutex;
static const uint CONCURRENCY = thread::hardware_concurrency();
#endif

// Convenience function that prints Options.hpp settings
static void printRenderingOptions()
{

#ifdef ENABLE_MULTITHREADING
	cout << "Multithreading enabled" << endl;
#else
	cout << "Multithreading disabled. " << endl;
#endif

#ifdef ENABLE_BOUNDING_VOLUMES
	string boundingVolumeNames[2] = {
		"Bounding Box",
		"Bounding Sphere"
	};

	cout << "Bounding volume acceleration enabled (" << boundingVolumeNames[BOUNDING_VOLUME] << ")" << endl;
	#ifdef ENABLE_BOUNDING_VOLUMES
		cout << "Rendering bounding volumes" << endl;
	#endif
#else
	cout << "Bounding volume acceleration disabled" << endl;
#endif

#ifdef ENABLE_SUPERSAMPLING
	cout << "Supersampling enabled" << endl;
#endif

#ifdef ENABLE_REFLECTIONS
	cout << "Reflections enabled" << endl;
#endif
}

// Safely increments the number of pixels rendered and updates the progress indicator
#ifdef SHOW_PROGRESS
static void updateProgress(
	const pair<size_t, size_t> &pixelDim,
	uint &pixelsRendered,
	const uint increment
)
{
#ifdef ENABLE_MULTITHREADING
	std::lock_guard<std::mutex> lock(ioMutex);
#endif

	pixelsRendered += increment;
	cout << "\r" << std::fixed << std::setprecision(2)
		 << float(pixelsRendered) / (pixelDim.first * pixelDim.second) * 100.0f 
		 << "% done" << std::flush;
}
#endif

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
	mat4 T1 = glm::translate(vec3(-n_x * 0.5, -n_y * 0.5, d));

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
	const vec3 &ambient,
	const list<Light *> &lights,
	const uint hitsLeft
)
{
	HitRecord rec = node->hit(r, EPSILON, INF_DOUBLE);

	// Hit, compute shadow rays
	if(rec.hit)
		return directColour(node, r, rec, ambient, lights, hitsLeft);	

	// No hit, use background colour
	else{
		const float t = 0.7f*(glm::normalize(r.direction).y + 1.0f);
		return vec3(1.0f-t) * DuskColour + t * ZenithColour;
	}
}

vec3 directColour(
	SceneNode *node,
	const Ray &primRay,
	const HitRecord &primRec,
	const vec3 &ambient,
	const list<Light *> &lights,
	const uint hitsLeft
)
{
	// Material
	const auto mat = static_cast<PhongMaterial *>(primRec.mat);

	// Diffuse, specular, and shininess components
	const auto &kd = mat->diffuse();
	const auto &ks = mat->specular();
	const auto &ke = mat->shininess();

	// Ambient component
	vec3 col = kd * ambient;

	const vec4 &d = primRay.direction;                 // Primary ray direction
	const vec4 n = glm::normalize(primRec.n);          // Intersection point normal (normalized)
	const vec4 p = primRec.point + CORRECTION * n;     // Intersection point (corrected)
	const vec4 v = glm::normalize(primRay.origin - p); // Intersection to eye point vector

	// Compute shadow rays
	for(const auto light : lights){
		const Ray shadowRay(p, vec4(light->position, 1) - p);

		// Shade pixel if shadow ray isn't obstructed
		HitRecord rec = node->hit(shadowRay, EPSILON, INF_DOUBLE);
		if(!rec.hit){
			// Blinn-Phong Shading
			const vec3 &I = light->colour;
			const double *falloff = light->falloff;

			vec4 l = glm::normalize(shadowRay.direction); // Light direction
			vec4 h = glm::normalize(v + l);               // Halfway vector

			double shadowRayLen = glm::length(shadowRay.direction);
			double attenuation = 1.0 / (falloff[0] + falloff[1] * shadowRayLen + falloff[2] * shadowRayLen * shadowRayLen);

			// Diffuse component
			col += kd * I * std::max(0.0f, glm::dot(n, l)) * attenuation;

			// Specular component
			col += ks * I * std::pow(std::max(0.0f, glm::dot(n, h)), ke) * attenuation;
		}
	}

	// Reflect light off of anything except the ground plane
	// Note: Check for ground plane is hacky
#ifdef ENABLE_REFLECTIONS
	if(hitsLeft > 0 && *primRec.name != "plane"){
		const auto r = glm::reflect(d, n); // Reflection direction
		const Ray reflectedRay(p, r);
		const vec3 reflectionCol = rayColour(node, reflectedRay, ambient, lights, hitsLeft-1);
		col = glm::mix(col, reflectionCol, REFLECTION_MIX_FACTOR);
	}
#endif

	return col;
}

static void renderChunk(
	const pair<size_t, size_t> &pixelDim,
	const uint xStart, const uint xEnd,

	Image &image,

	SceneNode *root,

	const mat4 &dcsToWorld,
	const vec4 &eye,

	const vec3 & ambient,
	const list<Light *> & lights,

	uint &pixelsRendered
)
{
	const uint n_y = pixelDim.second;

	for(uint x = xStart; x < xEnd; ++x) {
		for(uint y = 0; y < n_y; ++y){
			vec3 col(0.0f);                // Pixel colour
			vec4 p_dcs = vec4(x, y, 0, 1); // Pixel position (DCS)

		// Supersample
#ifdef ENABLE_SUPERSAMPLING
			double SS_INV = 1.0 / SS_FACTOR;

			for(uint u = 0; u < SS_FACTOR; ++u){
				for(uint v = 0; v < SS_FACTOR; ++v){
					p_dcs.x = x + double(u) * SS_INV;
					p_dcs.y = y + double(v) * SS_INV;
#endif
					const vec4 p_world = dcsToWorld * p_dcs; // Pixel position (WCS)
					const Ray ray(eye, p_world - eye);

					// Compute pixel colour
					col += rayColour(root, ray, ambient, lights, MAX_HITS);

#ifdef ENABLE_SUPERSAMPLING
				}
			}

			// Average sampled pixel colours
			col *= SS_INV * SS_INV;
#endif

			// Red: 
			image(x, y, Cone::R) = col[Cone::R];
			// Green: 
			image(x, y, Cone::G) = col[Cone::G];
			// Blue: 
			image(x, y, Cone::B) = col[Cone::B];
		}

#ifdef SHOW_PROGRESS
		updateProgress(pixelDim, pixelsRendered, n_y);
#endif
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
	cout <<")" << endl << endl;

	// Image dimensions
	const size_t n_x = image.width();
	const size_t n_y = image.height();
	const auto pixelDim = std::make_pair(n_x, n_y);

	// DCS to WCS matrix
	const mat4 dcsToWorld = generateDCStoWorldMat(pixelDim, eye, view, up, fovy);

	// Eye 4D point
	const vec4 eye4D(eye, 1);

	/* Ray Trace image */
	printRenderingOptions();

	// Start rendering!
	{
		Timer timer;

		uint pixelsRendered = 0;

#ifdef ENABLE_MULTITHREADING
		const uint numWorkers = CONCURRENCY;
		const uint chunkSize = 1 + ((n_x - 1) / numWorkers);

		cout << endl << "Multithreading settings: " << endl;
			cout << "\t" << numWorkers << " workers" << endl;
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
				std::ref(pixelDim),
				xStart, xEnd,
				std::ref(image),
				root,
				std::ref(dcsToWorld),
				std::ref(eye4D),
				std::ref(ambient),
				std::ref(lights),
				std::ref(pixelsRendered)
			));
		}

#else
		for(uint x = 0; x < n_x; ++x)
			renderChunk(pixelDim, x, x+1, image, root, dcsToWorld, eye4D, ambient, lights, pixelsRendered);
#endif
	}
}
