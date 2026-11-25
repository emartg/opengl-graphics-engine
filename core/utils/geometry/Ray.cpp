/*
 * Ray.cpp
 * This file implements the Ray struct, a half-line from an origin along a unit direction.
 */

#include "Ray.h"

#include <cmath>

// Public Methods
// --------------
std::optional<float> Ray::intersect_horizontal_plane(float height) const
{
	// a ray parallel to the plane (or almost) never reaches it, and a ray that does not go towards it misses it
	if (std::abs(direction.y) < 1e-6f)
		return std::nullopt;
	const float distance = (height - origin.y) / direction.y;
	if (!(distance >= 0.0f)) // also false for a distance that is not a number
		return std::nullopt;
	return distance;
}
