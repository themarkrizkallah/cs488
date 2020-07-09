#pragma once

#include <limits>
#include <math.h>

const double INF_DOUBLE = std::numeric_limits<double>::infinity();
const double EPSILON = 0.001;
const double FUDGE_FACTOR = 0.005;

inline bool almostEquals(double l, double r, double epsilon = EPSILON)
{
    return std::abs(l - r) < epsilon;
}

inline bool significant(double x, double epsilon = EPSILON)
{
    return std::abs(x) >= epsilon;
}
