// Spring 2020

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <utility>

#include "Material.hpp"
#include "PhongMaterial.hpp"
#include "Common.hpp"
#include "A4.hpp"

using namespace std;
using namespace glm;


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
	const double d = 1;

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
	const glm::vec3 &ambient,
	const std::list<Light *> &lights
)
{
	vec3 col;
	HitRecord rec = node->hit(r, mat4());

	// Hit, compute shadow rays
	if(rec.hit)
		return directColour(node, r, rec, ambient, lights);	

	// No hit, use background colour
	else
		return BackgroundColour;

	return col;
}

vec3 directColour(
	SceneNode *node,
	const Ray &primRay,
	const HitRecord &primRec,
	const glm::vec3 &ambient,
	const std::list<Light *> &lights
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

	const vec4 n = glm::normalize(primRec.n); // Intersection normal (normalized)
	const vec4 p = primRec.point + FUDGE_FACTOR * n; // Intersection point (corrected)
	const vec4 v = glm::normalize(primRay.origin - p);

	// Compute shadow rays
	for(const auto light : lights){
		const Ray shadowRay(p, vec4(light->position, 1));
		const HitRecord rec = node->hit(shadowRay, mat4());

		// If shadow ray not obstructed
		if(rec.hit == false){
			// Blinn-Phong Shading
			const vec3 &I = light->colour;

			vec4 l = shadowRay.direction(); // Light direction
			vec4 h = glm::normalize(v + l); // Halfway vector

			// Diffuse component
			col += kd * I * std::max(0.0f, glm::dot(n, l));

			// Specular component
			col += ks * I * std::pow(std::max(0.0f, glm::dot(n, h)), ke);
		}
	}

	return col;
}

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
) {

  // Fill in raytracing code here...  
  std::cout << "Calling A4_Render(\n" <<
		  "\t" << *root <<
          "\t" << "Image(width:" << image.width() << ", height:" << image.height() << ")\n"
          "\t" << "eye:  " << glm::to_string(eye) << std::endl <<
		  "\t" << "view: " << glm::to_string(view) << std::endl <<
		  "\t" << "up:   " << glm::to_string(up) << std::endl <<
		  "\t" << "fovy: " << fovy << std::endl <<
          "\t" << "ambient: " << glm::to_string(ambient) << std::endl <<
		  "\t" << "lights{" << std::endl;

	for(const Light * light : lights) {
		std::cout << "\t\t" <<  *light << std::endl;
	}
	std::cout << "\t}" << std::endl;
	std:: cout <<")" << std::endl;

	// Image dimensions
	const size_t n_x = image.width();
	const size_t n_y = image.height();
	const auto pixelDim = std::make_pair(n_x, n_y);

	const mat4 dcsToWorld = generateDCStoWorldMat(pixelDim, eye, view, up, fovy);

	// Cast rays
	for (uint x = 0; x < n_x; ++x) {
		for (uint y = 0; y < n_y; ++y) {
			const vec4 p_world = dcsToWorld * glm::vec4(x, y, 0, 1);
			const Ray ray(vec4(eye, 1), p_world);

			// Compute pixel colour
			auto col = rayColour(root, ray, 0, ambient, lights);

			// Red: 
			image(x, y, Cone::R) = col[Cone::R];
			// Green: 
			image(x, y, Cone::G) = col[Cone::G];
			// Blue: 
			image(x, y, Cone::B) = col[Cone::B];
		}
	}
}
