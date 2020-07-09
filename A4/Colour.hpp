#pragma once

#include <glm/glm.hpp>

enum Cone {
	R = 0,
	G,
	B
};

struct Colour{
	Colour(double r = 0.0, double g = 0.0, double b = 0.0);
	Colour(const Colour &other);
	Colour(glm::vec3 colour);

	double col[3];

	double &operator[](int i);
	double &operator[](Cone i);

	Colour operator+(Colour colour);
	Colour operator+(glm::vec3 colour);
	Colour &operator+=(Colour colour);
	Colour &operator+=(glm::vec3 colour);
};