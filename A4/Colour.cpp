#include "Colour.hpp"

#include <glm/glm.hpp>

using namespace glm;

// Default constructor
Colour::Colour(double r, double b, double g)
{ 
    col[Cone::R] = r; 
    col[Cone::G] = g; 
    col[Cone::B] = b;
}

Colour::Colour(const Colour &other)
{
    col[Cone::R] = other.col[Cone::R]; 
    col[Cone::G] = other.col[Cone::G]; 
    col[Cone::B] = other.col[Cone::B];
}

// Construct from vec3
Colour::Colour(vec3 colour)
{ 
    col[Cone::R] = colour[Cone::R]; 
    col[Cone::G] = colour[Cone::G]; 
    col[Cone::B] = colour[Cone::B];
}

double &Colour::operator[](int i)
{ 
    return col[i];
}

double &Colour::operator[](Cone i)
{
    return col[i];
}

Colour Colour::operator+(Colour colour)
{
    col[Cone::R] += colour[Cone::R]; 
    col[Cone::G] += colour[Cone::G]; 
    col[Cone::B] += colour[Cone::B];

    return *this;
}

Colour Colour::operator+(glm::vec3 colour)
{
    col[Cone::R] += colour[Cone::R]; 
    col[Cone::G] += colour[Cone::G]; 
    col[Cone::B] += colour[Cone::B];

    return *this;
}

Colour &Colour::operator+=(Colour colour)
{
    col[Cone::R] += colour[Cone::R]; 
    col[Cone::G] += colour[Cone::G]; 
    col[Cone::B] += colour[Cone::B];

    return *this;
}

Colour &Colour::operator+=(glm::vec3 colour)
{
    col[Cone::R] += colour[Cone::R]; 
    col[Cone::G] += colour[Cone::G]; 
    col[Cone::B] += colour[Cone::B];

    return *this;
}