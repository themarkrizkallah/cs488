// Spring 2020

#include "PhongMaterial.hpp"

using namespace glm;

PhongMaterial::PhongMaterial(
	const vec3& kd, const vec3& ks, double shininess )
	: m_kd(kd)
	, m_ks(ks)
	, m_shininess(shininess)
{}

PhongMaterial::~PhongMaterial()
{}

vec3 PhongMaterial::diffuse()
{
	return m_kd;
}

vec3 PhongMaterial::specular()
{
	return m_ks;
}

double PhongMaterial::shininess()
{
	return m_shininess;
}
