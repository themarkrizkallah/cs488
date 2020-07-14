// Spring 2020

// #include "cs488-framework/ObjFileDecoder.hpp"
#include "Options.hpp"
#include "Epsilon.hpp"
#include "Ray.hpp"
#include "Mesh.hpp"

#include <iostream>
#include <fstream>
#include <memory>

#include <glm/ext.hpp>

using namespace std;
using namespace glm;

// Fundamentals of Computer Graphics 4.4.2
HitRecord Triangle::hit(const Ray &r, double t0, double t1, const vec3 *verts) const
{
	HitRecord rec;

	// Vertices
	const auto &v1 = verts[0];
	const auto &v2 = verts[1];
	const auto &v3 = verts[2];

	// Column 3 of A, [g, h, i]^T
	const auto direction = r.direction;
	const double g = direction.x;
	const double h = direction.y;
	const double i = direction.z;

	// Column 1
	const double a = v1.x - v2.x;
	const double b = v1.y - v2.y;
	const double c = v1.z - v2.z;

	// Column 2
	const double d = v1.x - v3.x;
	const double e = v1.y - v3.y;
	const double f = v1.z - v3.z;

	// Right hand side
	const double j = v1.x - r.origin.x;
	const double k = v1.y - r.origin.y;
	const double l = v1.z - r.origin.z;

	// Cache reused computations
	const double ei_minus_hf = (e * i) - (h * f);
	const double gf_minus_di = (g * f) - (d * i);
	const double dh_minus_eg = (d * h) - (e * g);
	const double ak_minus_jb = (a * k) - (j * b);
	const double jc_minus_al = (j * c) - (a * l);
	const double bl_minus_kc = (b * l) - (k * c);

	// Compute M, invert it here and then multiply moving forward
	const double M = 1.0 / ((a * ei_minus_hf) + (b * gf_minus_di) + (c * dh_minus_eg));

	// Compute t and verify that it is in bounds
	double t = - ((f * ak_minus_jb) + (e * jc_minus_al) + (d * bl_minus_kc)) * M;
	if(t <= t0 || t >= t1 || t >= rec.t)
		return rec;

	// Compute gamma and verify that it is in bounds
	double gamma = ((i * ak_minus_jb) + (h * jc_minus_al) + (g * bl_minus_kc)) * M;
	if(gamma < EPSILON || gamma > 1.0)
		return rec;

	// Compute Beta and verify that it is in bounds
	double beta = ((j * ei_minus_hf) + (k * gf_minus_di) + (l * dh_minus_eg)) * M;
	if(beta < EPSILON || beta > 1.0 - gamma)
		return rec;

	// Intersected triangle at a closer point, update rec
	rec.hit = true;
	rec.t = t;
	rec.n = vec4(glm::cross(v2 - v1, v3 - v1), 0);

	return rec;
}


// ------------------------------------------------------------
// Mesh
Mesh::Mesh(const string &fname)
	: m_vertices(), 
	  m_faces()
#ifdef ENABLE_BOUNDING_VOLUMES
	  ,m_boundingMin(INF_FLOAT), 
	  m_boundingMax(-INF_FLOAT)
#endif
{
	string code;
	double vx, vy, vz;
	size_t s1, s2, s3;

	ifstream ifs(("Assets/" + fname).c_str());

	while( ifs >> code ) {
		if(code == "v") {
			ifs >> vx >> vy >> vz;
			m_vertices.emplace_back(vx, vy, vz);

#ifdef ENABLE_BOUNDING_VOLUMES
			// Find min and max points
			if(vx < m_boundingMin.x) m_boundingMin.x = vx;
			if(vx > m_boundingMax.x) m_boundingMax.x = vx;

			if(vy < m_boundingMin.y) m_boundingMin.y = vy;
			if(vy > m_boundingMax.y) m_boundingMax.y = vy;

			if(vz < m_boundingMin.z) m_boundingMin.z = vz;
			if(vz > m_boundingMax.z) m_boundingMax.z = vz;
#endif

		} else if(code == "f") {
			ifs >> s1 >> s2 >> s3;
			m_faces.emplace_back(s1 - 1, s2 - 1, s3 - 1);
		}
	}

#ifdef ENABLE_BOUNDING_VOLUMES
	// Generate bounding volume
	m_bv = unique_ptr<Primitive>(boundingVolume(BOUNDING_VOLUME));
#endif
}

#ifdef ENABLE_BOUNDING_VOLUMES
Primitive *Mesh::boundingVolume(BoundingVolume volType) const
{
	Primitive *volume;

	switch(volType){
		// Generate a bounding sphere
		case BoundingVolume::BoundingSphere:
			volume = new NonhierSphere(
				(m_boundingMax + m_boundingMin) * 0.5f, 
				glm::length(m_boundingMax - m_boundingMin) * 0.5f
			);
			break;

		// (Default) Generate a bounding box
		case BoundingVolume::BoundingBox:
		default:
			volume = new NonhierBox(m_boundingMin, m_boundingMax - m_boundingMin);;
			break;
	}
	
	return volume;
}
#endif

HitRecord Mesh::hit(const Ray &r, double t0, double t1) const
{
	HitRecord rec;

#ifdef ENABLE_BOUNDING_VOLUMES
	// Check intersection with bounding volume (and possibly render it)
	#ifdef RENDER_BOUNDING_VOLUMES
		return m_bv->hit(r, t0, t1);
	#else
		if(!m_bv->hit(r, t0, t1))
			return rec;
	#endif
#endif
	const auto &direction = r.direction;

	// Column 3 of A, [g, h, i]^T
	const double g = direction.x;
	const double h = direction.y;
	const double i = direction.z;

	for(const auto triangle : m_faces){		
		const vec3 triangleVerts[3] = {
			m_vertices[triangle.v1],
			m_vertices[triangle.v2],
			m_vertices[triangle.v3]
		};

		// Check if triangle is hit
		HitRecord record = triangle.hit(r, t0, t1, triangleVerts);
		if(record.hit){
			t1 = record.t;
			rec = record;
		}
	}

	if(rec.hit)
		rec.point = r.pointAt(rec.t);

	return rec;
}

std::ostream& operator<<(ostream& out, const Mesh& mesh)
{
  out << "mesh {";
//   /*
  
//   for( size_t idx = 0; idx < mesh.m_verts.size(); ++idx ) {
//   	const MeshVertex& v = mesh.m_verts[idx];
//   	out << glm::to_string( v.m_position );
// 	if( mesh.m_have_norm ) {
//   	  out << " / " << glm::to_string( v.m_normal );
// 	}
// 	if( mesh.m_have_uv ) {
//   	  out << " / " << glm::to_string( v.m_uv );
// 	}
//   }

// */
  out << "}";
  return out;
}
